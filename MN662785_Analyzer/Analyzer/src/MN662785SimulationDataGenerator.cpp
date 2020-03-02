#include "MN662785SimulationDataGenerator.h"
#include "MN662785AnalyzerSettings.h"

MN662785SimulationDataGenerator::MN662785SimulationDataGenerator()
{
}

MN662785SimulationDataGenerator::~MN662785SimulationDataGenerator()
{
}

void MN662785SimulationDataGenerator::Initialize(U32 simulation_sample_rate, MN662785AnalyzerSettings *settings)
{
    mSimulationSampleRateHz = simulation_sample_rate;
    mSettings = settings;

    mClockGenerator.Init(mSettings->mBitRate, simulation_sample_rate);
    mMN662785SimulationData.SetChannel(mSettings->mInputChannel);
    mMN662785SimulationData.SetSampleRate(simulation_sample_rate);

    if (mSettings->mInverted == false) {
        mBitLow = BIT_LOW;
        mBitHigh = BIT_HIGH;
    } else {
        mBitLow = BIT_HIGH;
        mBitHigh = BIT_LOW;
    }

    mMN662785SimulationData.SetInitialBitState(mBitHigh);
    mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(10.0));  //insert 10 bit-periods of idle

    mValue = 0;

    mMpModeAddressMask = 0;
    mMpModeDataMask = 0;
    mNumBitsMask = 0;

    U32 num_bits = mSettings->mBitsPerTransfer;
    for (U32 i = 0; i < num_bits; i++) {
        mNumBitsMask <<= 1;
        mNumBitsMask |= 0x1;
    }

    if (mSettings->mMN662785Mode == MN662785AnalyzerEnums::MpModeMsbOneMeansAddress) {
        mMpModeAddressMask = 0x1ull << (mSettings->mBitsPerTransfer);
    }

    if (mSettings->mMN662785Mode == MN662785AnalyzerEnums::MpModeMsbZeroMeansAddress) {
        mMpModeDataMask = 0x1ull << (mSettings->mBitsPerTransfer);
    }
}

U32 MN662785SimulationDataGenerator::GenerateSimulationData(U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample(largest_sample_requested, sample_rate, mSimulationSampleRateHz);

    while (mMN662785SimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested) {
        if (mSettings->mMN662785Mode == MN662785AnalyzerEnums::Normal) {
            CreateMN662785Byte(mValue++);

            mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(10.0));     //insert 10 bit-periods of idle
        } else {
            U64 address = 0x1 | mMpModeAddressMask;
            CreateMN662785Byte(address);

            for (U32 i = 0; i < 4; i++) {
                mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(2.0));  //insert 2 bit-periods of idle
                CreateMN662785Byte((mValue++ & mNumBitsMask) | mMpModeDataMask);
            };

            mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(20.0));     //insert 20 bit-periods of idle

            address = 0x2 | mMpModeAddressMask;
            CreateMN662785Byte(address);

            for (U32 i = 0; i < 4; i++) {
                mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(2.0));  //insert 2 bit-periods of idle
                CreateMN662785Byte((mValue++ & mNumBitsMask) | mMpModeDataMask);
            };

            mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(20.0));     //insert 20 bit-periods of idle

        }
    }

    *simulation_channels = &mMN662785SimulationData;


    return 1;  // we are retuning the size of the SimulationChannelDescriptor array.  In our case, the "array" is length 1.
}

void MN662785SimulationDataGenerator::CreateMN662785Byte(U64 value)
{
    //assume we start high

    mMN662785SimulationData.Transition();  //low-going edge for start bit
    mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod());    //add start bit time

    if (mSettings->mInverted == true) {
        value = ~value;
    }

    U32 num_bits = mSettings->mBitsPerTransfer;
    if (mSettings->mMN662785Mode != MN662785AnalyzerEnums::Normal) {
        num_bits++;
    }

    BitExtractor bit_extractor(value, mSettings->mShiftOrder, num_bits);

    for (U32 i = 0; i < num_bits; i++) {
        mMN662785SimulationData.TransitionIfNeeded(bit_extractor.GetNextBit());
        mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod());
    }

    if (mSettings->mParity == AnalyzerEnums::Even) {

        if (AnalyzerHelpers::IsEven(AnalyzerHelpers::GetOnesCount(value)) == true) {
            mMN662785SimulationData.TransitionIfNeeded(mBitLow);    //we want to add a zero bit
        } else {
            mMN662785SimulationData.TransitionIfNeeded(mBitHigh);    //we want to add a one bit
        }

        mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod());

    } else if (mSettings->mParity == AnalyzerEnums::Odd) {

        if (AnalyzerHelpers::IsOdd(AnalyzerHelpers::GetOnesCount(value)) == true) {
            mMN662785SimulationData.TransitionIfNeeded(mBitLow);    //we want to add a zero bit
        } else {
            mMN662785SimulationData.TransitionIfNeeded(mBitHigh);
        }

        mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod());

    }

    mMN662785SimulationData.TransitionIfNeeded(mBitHigh);   //we need to end high

    //lets pad the end a bit for the stop bit:
    mMN662785SimulationData.Advance(mClockGenerator.AdvanceByHalfPeriod(mSettings->mStopBits));
}
