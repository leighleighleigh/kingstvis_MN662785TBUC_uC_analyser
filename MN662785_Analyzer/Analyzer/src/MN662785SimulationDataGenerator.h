#ifndef MN662785_SIMULATION_DATA_GENERATOR
#define MN662785_SIMULATION_DATA_GENERATOR

#include <AnalyzerHelpers.h>

class MN662785AnalyzerSettings;

class MN662785SimulationDataGenerator
{
public:
    MN662785SimulationDataGenerator();
    ~MN662785SimulationDataGenerator();

    void Initialize(U32 simulation_sample_rate, MN662785AnalyzerSettings *settings);
    U32 GenerateSimulationData(U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor **simulation_channels);

protected:
    MN662785AnalyzerSettings *mSettings;
    U32 mSimulationSampleRateHz;
    // BitState mBitLow;
    // BitState mBitHigh;
    // U64 mValue;

protected: //MN662785 specific

    void WriteMN662785Byte(U8 data);
    void CreateMN662785Sequence();
    ClockGenerator mClockGenerator;
    SimulationChannelDescriptorGroup mMN662785SimulationChannels;
    SimulationChannelDescriptor *mData;
    SimulationChannelDescriptor *mClock;
    SimulationChannelDescriptor *mLoad;
};

#endif //UNIO_SIMULATION_DATA_GENERATOR
