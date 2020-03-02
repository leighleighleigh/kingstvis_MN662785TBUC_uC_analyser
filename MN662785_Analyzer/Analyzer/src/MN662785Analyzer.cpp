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

void MN662785Analyzer::SetupResults()
{
    //Unlike the worker thread, this function is called from the GUI thread
    //we need to reset the Results object here because it is exposed for direct access by the GUI, and it can't be deleted from the WorkerThread

    mResults.reset(new MN662785AnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());

    if (mSettings->mDataChannel != UNDEFINED_CHANNEL)
    {
        mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel);
    }
}

void MN662785Analyzer::WorkerThread()
{
    // Make a new results reference, like in the GUI.
    mResults.reset(new MN662785AnalyzerResults(this, mSettings.get()));
    SetAnalyzerResults(mResults.get());

    if (mSettings->mDataChannel != UNDEFINED_CHANNEL)
    {
        mResults->AddChannelBubblesWillAppearOn(mSettings->mDataChannel);
    }

    // Get the current sampling rate.
    mSampleRateHz = GetSampleRate();

    // Get all the data!
    mData = GetAnalyzerChannelData(mSettings->mDataChannel);
    mClock = GetAnalyzerChannelData(mSettings->mClockChannel);
    mLoad = GetAnalyzerChannelData(mSettings->mLoadChannel);


    // Check we aren't starting with tclock low
    if(mClock->GetBitState() == BIT_LOW)
    {
        // Go to next edge
        mClock->AdvanceToNextEdge();
    }

    // Lets make a frame
    Frame frame;
    uint8_t bitCount = 0;

    // Work through and indicate the rising clock edges, with markers.
    for (;;)
    {
        mClock->AdvanceToNextEdge();

        if (mClock->GetBitState() == BIT_HIGH)
        {
            // Move mData to this position
            mData->AdvanceToAbsPosition(mClock->GetSampleNumber());
            uint8_t dataVal = (mData->GetBitState() == BIT_HIGH) ? 1 : 0;

            // Place a market here
            mResults->AddMarker(mClock->GetSampleNumber(), AnalyzerResults::MarkerType::UpArrow, mSettings->mClockChannel);

            // Increment the bit thing
            bitCount ++;

            // Start building frame
            if (bitCount == 1)
            {
                frame.mStartingSampleInclusive = mClock->GetSampleNumber();
                frame.mData1 = 0x0 | dataVal;
                frame.mFlags = 0;                
            }
            
            if(bitCount > 1 && bitCount < 8)
            {
                frame.mData1 = (frame.mData1 << 1) | dataVal;
            }

            if(bitCount == 8)
            {
                frame.mData1 = (frame.mData1 << 1) | dataVal;
                frame.mEndingSampleInclusive = mClock->GetSampleNumber();
                mResults->AddFrame(frame);
                mResults->CommitResults();
                ReportProgress(frame.mEndingSampleInclusive);

                // Reset bit thing
                bitCount = 0;
            }
        }

        CheckIfThreadShouldExit();
    }
}

bool MN662785Analyzer::NeedsRerun()
{
    return false;
}

U32 MN662785Analyzer::GenerateSimulationData(U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor **simulation_channels)
{
    if (mSimulationInitilized == false)
    {
        mSimulationDataGenerator.Initialize(GetSimulationSampleRate(), mSettings.get());
        mSimulationInitilized = true;
    }

    return mSimulationDataGenerator.GenerateSimulationData(minimum_sample_index, device_sample_rate, simulation_channels);
}

U32 MN662785Analyzer::GetMinimumSampleRateHz()
{
    return 400000 * 4;
}

const char *MN662785Analyzer::GetAnalyzerName() const
{
    return "MN662785";
}

const char *GetAnalyzerName()
{
    return "MN662785";
}

Analyzer *CreateAnalyzer()
{
    return new MN662785Analyzer();
}

void DestroyAnalyzer(Analyzer *analyzer)
{
    delete analyzer;
}
