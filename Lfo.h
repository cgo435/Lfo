#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct LfoTable {
    const int numPoints = 2048;
    const int numChannels = 4;
    
    LfoTable();
    ~LfoTable();
    
    const dsp::LookupTable<float>* operator [](String waveId) const;
    const dsp::LookupTable<float>* operator [](int waveIndex) const;
    
    void addShapeFromFile(File wavFile, String name);
    
private:
    void readShapeFromAfr(AudioFormatReader* reader);
    void addShapeFromBinary(const char* resourceName, String name);
    
    OwnedArray<dsp::LookupTable<float>> luts;
    Array<String> names;
    
};


class Lfo
{
public:
    Lfo(const LfoTable& table, float sr);
    ~Lfo();
    
    void setFrequency(float frequency);
    
    void setWaveType(String waveId);
    void setWaveType(int waveIndex);
    
    forcedinline float getNextSample();
    void fillBuffer(float* buffer, int numSamples);
    
    void reset();
    
    float phaseOffset = 0.0f;

private:
    const LfoTable& lfoTable;
    const dsp::LookupTable<float>* currentTable;
    
    float sampleRate;
    float readIndex;
    float increment;
    
    int numPoints;
    
};
