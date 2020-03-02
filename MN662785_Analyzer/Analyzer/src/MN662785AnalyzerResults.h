#ifndef MN662785_ANALYZER_RESULTS
#define MN662785_ANALYZER_RESULTS

#include <AnalyzerResults.h>

#define FRAMING_ERROR_FLAG ( 1 << 0 )
#define PARITY_ERROR_FLAG ( 1 << 1 )
#define MP_MODE_ADDRESS_FLAG ( 1 << 2 )

class MN662785Analyzer;
class MN662785AnalyzerSettings;

class MN662785AnalyzerResults : public AnalyzerResults
{
public:
    MN662785AnalyzerResults(MN662785Analyzer *analyzer, MN662785AnalyzerSettings *settings);
    virtual ~MN662785AnalyzerResults();

    virtual void GenerateBubbleText(U64 frame_index, Channel &channel, DisplayBase display_base);
    virtual void GenerateExportFile(const char *file, DisplayBase display_base, U32 export_type_user_id);

    virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base);
    virtual void GeneratePacketTabularText(U64 packet_id, DisplayBase display_base);
    virtual void GenerateTransactionTabularText(U64 transaction_id, DisplayBase display_base);

protected: //functions

protected:  //vars
    MN662785AnalyzerSettings *mSettings;
    MN662785Analyzer *mAnalyzer;
};

#endif //MN662785_ANALYZER_RESULTS
