#pragma once 
#include <Fuzzy.h>

class FuzzyHidroponik
{
    private:
        int num;
        FuzzyInput* ph;
        Fuzzy* fuzzy;

        // FuzzySet
        FuzzySet* phAsam;
        FuzzySet* phNetral;
        FuzzySet* phBasa;

        FuzzySet* tdsLow;
        FuzzySet* tdsNormal;
        FuzzySet* tdsHigh;

        FuzzySet* tempDingin;
        FuzzySet* tempIdeal;
        FuzzySet* tempPanas;

        FuzzySet* nutrisiRendah;
        FuzzySet* nutrisiSedang;
        FuzzySet* nutrisiTinggi;

        FuzzySet* coolerOff;
        FuzzySet* coolerOn;

        FuzzySet* pengencerOff;
        FuzzySet* pengencerOn;

    public:
        FuzzyHidroponik();
        void begin();
        void setInput(int input, float value);
        void fuzzify();
        int defuzzify(int output);
};