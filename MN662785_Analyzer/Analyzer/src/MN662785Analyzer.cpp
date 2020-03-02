#include "MN662785Analyzer.h"
#include "MN662785AnalyzerSettings.h"
#include <AnalyzerChannelData.h>

MN662785Analyzer::MN662785Analyzer()
    : Analyzer(),
      mSettings(new MN662785AnalyzerSettings()),
      mSimulationInitilized(false)
{
    SetAnalyzerSettings(mSettings.get());
}

MN662785Analyzer::~MN662785Analyzer()
{
    KillThread();
}

void MN662785Analyzer::ComputeSampleOffsets()
{
    ClockGenerator clock_generator;
    clock_generator.Init(mSettings->mBitRate, mSampleRateHz);

    mSampleOffsets.clear();

    U32 num_bits = mSettings->mBitsPerTransfer;

    if (mSettings->mMN662785Mode != MN662785AnalyzerEnums::Normal) {
        num_bits++;
    }

    mSampleOffsets.push_back(clock_generator.AdvanceByHalfPeriod(1.5));  //point to the center of the 1st bit (past the start bit)
    num_bits--;  //we just added the first bit.

    for (U32 i = 0; i < num_bits; i++) {
        mSampleOffsets.push_back(clock_generator.AdvanceByHalfPeriod());
    }

    if (mSettings->mParity != AnalyzerEnums::None) {
        mParityBitOffset = clock_generator.AdvanceByHalfPeriod();
    }

    //to check for framing errors, we also want to check
    //1/2 bit after the beginning of the stop bit
    mStartOfStopBitOffset = clock_generator.AdvanceByHalfPeriod(1.0);   //i.e. moving from the center of the last data bit (where we left off) to 1/2 period into the stop bit

    //and 1/2 bit before end of the stop bit period
    mEndOfStopBitOffset = clock_generator.AdvanceByHalfPeriod(mSettings->mStopBits - 1.0);  //if stopbits == 1.0, this will be 0
}

void MN662785Analyzer::SetupResults()
{
    //Unlike the worker thread, this function is called from the GUI thread
    //we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the WorkerThread

    mResults.reset(new MN662785AnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());
    mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

void MN662785Analyzer::WorkerThread()
{
    mSampleRateHz = GetSampleRate();
    ComputeSampleOffsets();
    U32 num_bits = mSettings->mBitsPerTransfer;

    if (mSettings->mMN662785Mode != MN662785AnalyzerEnums::Normal) {
        num_bits++;
    }

    if (mSettings->mInverted == false) {
        mBitHigh = BIT_HIGH;
        mBitLow = BIT_LOW;
    } else {
        mBitHigh = BIT_LOW;
        mBitLow = BIT_HIGH;
    }

    U64 bit_mask = 0;
    U64 mask = 0x1ULL;
    for (U32 i = 0; i < num_bits; i++) {
        bit_mask |= mask;
        mask <<= 1;
    }

    mMN662785 = GetAnalyzerChannelData(mSettings->mInputChannel);
    mMN662785->TrackMinimumPulseWidth();
    if (mMN662785->GetBitState() == mBitLow) {
        mMN662785->AdvanceToNextEdge();
    }

    for (; ;) {
        //we're starting high.  (we'll assume that we're not in the middle of a byte.)

        mMN662785->AdvanceToNextEdge();

        //we're now at the beginning of the start bit.  We can start collecting the data.
        U64 frame_starting_sample = mMN662785->GetSampleNumber();

        U64 data = 0;
        bool parity_error = false;
        bool framing_error = false;
        bool mp_is_address = false;

        DataBuilder data_builder;
        data_builder.Reset(&data, mSettings->mShiftOrder, num_bits);
        U64 marker_location = frame_starting_sample;

        for (U32 i = 0; i < num_bits; i++) {
            mMN662785->Advance(mSampleOffsets[i]);
            data_builder.AddBit(mMN662785->GetBitState());

            marker_location += mSampleOffsets[i];
            mResults->AddMarker(marker_location, AnalyzerResults::Dot, mSettings->mInputChannel);
        }
        if (mSettings->mInverted == true) {
            data = (~data) & bit_mask;
        }

        if (mSettings->mMN662785Mode != MN662785AnalyzerEnums::Normal) {
            //extract the MSB
            U64 msb = data >> (num_bits - 1);
            msb &= 0x1;
            if (mSettings->mMN662785Mode == MN662785AnalyzerEnums::MpModeMsbOneMeansAddress) {
                if (msb == 0x0) {
                    mp_is_address = false;
                } else {
                    mp_is_address = true;
                }
            }
            if (mSettings->mMN662785Mode == MN662785AnalyzerEnums::MpModeMsbZeroMeansAddress) {
                if (msb == 0x0) {
                    mp_is_address = true;
                } else {
                    mp_is_address = false;
                }
            }
            //now remove the msb.
            data &= (bit_mask >> 1);
        }

        parity_error = false;

        if (mSettings->mParity != AnalyzerEnums::None) {
            mMN662785->Advance(mParityBitOffset);
            bool is_even = AnalyzerHelpers::IsEven(AnalyzerHelpers::GetOnesCount(data));

            if (mSettings->mParity == AnalyzerEnums::Even) {
                if (is_even == true) {
                    if (mMN662785->GetBitState() != mBitLow) { //we expect a low bit, to keep the parity even.
                        parity_error = true;
                    }
                } else {
                    if (mMN662785->GetBitState() != mBitHigh) { //we expect a high bit, to force parity even.
                        parity_error = true;
                    }
                }
            } else { //if( mSettings->mParity == AnalyzerEnums::Odd )
                if (is_even == false) {
                    if (mMN662785->GetBitState() != mBitLow) { //we expect a low bit, to keep the parity odd.
                        parity_error = true;
                    }
                } else {
                    if (mMN662785->GetBitState() != mBitHigh) { //we expect a high bit, to force parity odd.
                        parity_error = true;
                    }
                }
            }

            marker_location += mParityBitOffset;
            mResults->AddMarker(marker_location, AnalyzerResults::Square, mSettings->mInputChannel);
        }

        //now we must dermine if there is a framing error.
        framing_error = false;

        mMN662785->Advance(mStartOfStopBitOffset);

        if (mMN662785->GetBitState() != mBitHigh) {
            framing_error = true;
        } else {
            U32 num_edges = mMN662785->Advance(mEndOfStopBitOffset);
            if (num_edges != 0) {
                framing_error = true;
            }
        }

        if (framing_error == true) {
            marker_location += mStartOfStopBitOffset;
            mResults->AddMarker(marker_location, AnalyzerResults::ErrorX, mSettings->mInputChannel);

            if (mEndOfStopBitOffset != 0) {
                marker_location += mEndOfStopBitOffset;
                mResults->AddMarker(marker_location, AnalyzerResults::ErrorX, mSettings->mInputChannel);
            }
        }

        //ok now record the value!
        //note that we're not using the mData2 or mType fields for anything, so we won't bother to set them.
        Frame frame;
        frame.mStartingSampleInclusive = frame_starting_sample;
        frame.mEndingSampleInclusive = mMN662785->GetSampleNumber();
        frame.mData1 = data;
        frame.mFlags = 0;
        if (parity_error == true) {
            frame.mFlags |= PARITY_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;
        }

        if (framing_error == true) {
            frame.mFlags |= FRAMING_ERROR_FLAG | DISPLAY_AS_ERROR_FLAG;
        }

        if (mp_is_address == true) {
            frame.mFlags |= MP_MODE_ADDRESS_FLAG;
        }

        if (mp_is_address == true) {
            mResults->CommitPacketAndStartNewPacket();
        }

        mResults->AddFrame(frame);

        mResults->CommitResults();

        ReportProgress(frame.mEndingSampleInclusive);
        CheckIfThreadShouldExit();

        if (framing_error == true) { //if we're still low, let's fix that for the next round.
            if (mMN662785->GetBitState() == mBitLow) {
                mMN662785->AdvanceToNextEdge();
            }
        }
    }
}

bool MN662785Analyzer::NeedsRerun()
{
    if (mSettings->mUseAutobaud == false) {
        return false;
    }

    //ok, lets see if we should change the bit rate, base on mShortestActivePulse

    U64 shortest_pulse = mMN662785->GetMinimumPulseWidthSoFar();

    if (shortest_pulse == 0) {
        AnalyzerHelpers::Assert("Alg problem, shortest_pulse was 0");
    }

    U32 computed_bit_rate = U32(double(mSampleRateHz) / double(shortest_pulse));

    if (computed_bit_rate > mSampleRateHz) {
        AnalyzerHelpers::Assert("Alg problem, computed_bit_rate is higer than sample rate");    //just checking the obvious...
    }

    if (computed_bit_rate > (mSampleRateHz / 4)) {
        return false;    //the baud rate is too fast.
    }
    if (computed_bit_rate == 0) {
        //bad result, this is not good data, don't bother to re-run.
        return false;
    }

    U32 specified_bit_rate = mSettings->mBitRate;

    double error = double(AnalyzerHelpers::Diff32(computed_bit_rate, specified_bit_rate)) / double(specified_bit_rate);

    if (error > 0.1) {
        mSettings->mBitRate = computed_bit_rate;
        mSettings->UpdateInterfacesFromSettings();
        return true;
    } else {
        return false;
    }
}

U32 MN662785Analyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    if (mSimulationInitilized == false) {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 MN662785Analyzer::GetMinimumSampleRateHz()
{
    return mSettings->mBitRate * 4;
}

const char *MN662785Analyzer::GetAnalyzerName() const
{
    return "UART/232/485";
}

const char *GetAnalyzerName()
{
    return "UART/232/485";
}

Analyzer *CreateAnalyzer()
{
    return new MN662785Analyzer();
}

void DestroyAnalyzer(Analyzer *analyzer)
{
    delete analyzer;
}

