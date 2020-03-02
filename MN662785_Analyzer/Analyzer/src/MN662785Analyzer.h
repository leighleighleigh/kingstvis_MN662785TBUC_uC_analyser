#ifndef MN662785_ANALYZER_H
#define MN662785_ANALYZER_H

#include <Analyzer.h>
#include "MN662785AnalyzerResults.h"
#include "MN662785SimulationDataGenerator.h"

class MN662785AnalyzerSettings;

class ANALYZER_EXPORT MN662785Analyzer : public Analyzer
{
public:
    MN662785Analyzer();
    virtual ~MN662785Analyzer();
    virtual void SetupResults();
    virtual void WorkerThread();

    virtual U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels);
    virtual U32 GetMinimumSampleRateHz();

    virtual const char *GetAnalyzerName() const;
    virtual bool NeedsRerun();


#pragma warning( push )
#pragma warning( disable : 4251 ) //warning C4251: 'MN662785Analyzer::<...>' : class <...> needs to have dll-interface to be used by clients of class

protected: //functions
    void ComputeSampleOffsets();

protected: //vars
    std::auto_ptr< MN662785AnalyzerSettings > mSettings;
    std::auto_ptr< MN662785AnalyzerResults > mResults;
    AnalyzerChannelData *mMN662785;

    MN662785SimulationDataGenerator mSimulationDataGenerator;
    bool mSimulationInitilized;

    //MN662785 analysis vars:
    U32 mSampleRateHz;
    std::vector<U32> mSampleOffsets;
    U32 mParityBitOffset;
    U32 mStartOfStopBitOffset;
    U32 mEndOfStopBitOffset;
    BitState mBitLow;
    BitState mBitHigh;

#pragma warning( pop )
};

extern "C" ANALYZER_EXPORT const char *__cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer *__cdecl CreateAnalyzer();
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer(Analyzer *analyzer);

#endif //MN662785_ANALYZER_H
