#include "FuzzyHidroponik.h"

FuzzyHidroponik::FuzzyHidroponik() {
    this->fuzzy = new Fuzzy();

    // FuzzySet pointers
    FuzzySet *phAsam, *phNetral, *phBasa;
    FuzzySet *tdsLow, *tdsNormal, *tdsHigh;
    FuzzySet *tempDingin, *tempIdeal, *tempPanas;
    FuzzySet *nutrisiRendah, *nutrisiSedang, *nutrisiTinggi;
    FuzzySet *coolerOff, *coolerOn;
    FuzzySet *pengencerOff, *pengencerOn;
}

void FuzzyHidroponik::begin() {
    this->ph = new FuzzyInput(1);
    phAsam = new FuzzySet(4, 4.5, 5.5, 6.5);
    phNetral = new FuzzySet(5.5, 6.5, 7.0, 7.5);
    phBasa = new FuzzySet(7.0, 8.0, 9.0, 9.5);
    ph->addFuzzySet(phAsam);
    ph->addFuzzySet(phNetral);
    ph->addFuzzySet(phBasa);
    fuzzy->addFuzzyInput(ph);

    // Input TDS
    FuzzyInput *tds = new FuzzyInput(2);
    tdsLow = new FuzzySet(0, 0, 10, 20);
    tdsNormal = new FuzzySet(15, 30, 50, 70);
    tdsHigh = new FuzzySet(60, 80, 100, 120);
    tds->addFuzzySet(tdsLow);
    tds->addFuzzySet(tdsNormal);
    tds->addFuzzySet(tdsHigh);
    fuzzy->addFuzzyInput(tds);

    // Input suhu
    FuzzyInput *temp = new FuzzyInput(3);
    tempDingin = new FuzzySet(15, 17, 19, 22);
    tempIdeal = new FuzzySet(21, 23, 26, 28);
    tempPanas = new FuzzySet(27, 30, 33, 35);
    temp->addFuzzySet(tempDingin);
    temp->addFuzzySet(tempIdeal);
    temp->addFuzzySet(tempPanas);
    fuzzy->addFuzzyInput(temp);

    // Output nutrisi (PWM)
    FuzzyOutput *nutrisi = new FuzzyOutput(1);
    nutrisiRendah = new FuzzySet(0, 0, 50, 100);
    nutrisiSedang = new FuzzySet(80, 120, 150, 180);
    nutrisiTinggi = new FuzzySet(170, 200, 255, 255);
    nutrisi->addFuzzySet(nutrisiRendah);
    nutrisi->addFuzzySet(nutrisiSedang);
    nutrisi->addFuzzySet(nutrisiTinggi);
    fuzzy->addFuzzyOutput(nutrisi);

    // Output pendingin (ON/OFF)
    FuzzyOutput *cooler = new FuzzyOutput(2);
    coolerOff = new FuzzySet(0, 0, 0, 0);
    coolerOn  = new FuzzySet(255, 255, 255, 255);
    cooler->addFuzzySet(coolerOff);
    cooler->addFuzzySet(coolerOn);
    fuzzy->addFuzzyOutput(cooler);

    // Output pengencer (ON/OFF)
    FuzzyOutput *diluter = new FuzzyOutput(3);
    pengencerOff = new FuzzySet(0, 0, 0, 0);
    pengencerOn  = new FuzzySet(255, 255, 255, 255);
    diluter->addFuzzySet(pengencerOff);
    diluter->addFuzzySet(pengencerOn);
    fuzzy->addFuzzyOutput(diluter);

    // Rules

    // R1: pH netral & TDS normal → Nutrisi sedang
    FuzzyRuleAntecedent *r1 = new FuzzyRuleAntecedent();
    r1->joinWithAND(phNetral, tdsNormal);
    FuzzyRuleConsequent *c1 = new FuzzyRuleConsequent();
    c1->addOutput(nutrisiSedang);
    fuzzy->addFuzzyRule(new FuzzyRule(1, r1, c1));

    // R2: pH netral & TDS low → Nutrisi tinggi
    FuzzyRuleAntecedent *r2 = new FuzzyRuleAntecedent();
    r2->joinWithAND(phNetral, tdsLow);
    FuzzyRuleConsequent *c2 = new FuzzyRuleConsequent();
    c2->addOutput(nutrisiTinggi);
    fuzzy->addFuzzyRule(new FuzzyRule(2, r2, c2));

    // R3: Suhu panas → Pendingin ON
    FuzzyRuleAntecedent *r3 = new FuzzyRuleAntecedent();
    r3->joinSingle(tempPanas);
    FuzzyRuleConsequent *c3 = new FuzzyRuleConsequent();
    c3->addOutput(coolerOn);
    fuzzy->addFuzzyRule(new FuzzyRule(3, r3, c3));

    // R4: TDS tinggi & pH asam → Pengencer ON
    FuzzyRuleAntecedent *r4 = new FuzzyRuleAntecedent();
    r4->joinWithAND(tdsHigh, phAsam);
    FuzzyRuleConsequent *c4 = new FuzzyRuleConsequent();
    c4->addOutput(pengencerOn);
    fuzzy->addFuzzyRule(new FuzzyRule(4, r4, c4));
}

void FuzzyHidroponik::setInput(int input, float value) {
    fuzzy->setInput(input, value);
}

void FuzzyHidroponik::fuzzify() {
    fuzzy->fuzzify();
}

int FuzzyHidroponik::defuzzify(int output) {
    return fuzzy->defuzzify(output);
}