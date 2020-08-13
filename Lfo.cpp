
#include "Lfo.h"

//==============================================================================


const dsp::LookupTable<float>* LfoTable::operator [](String waveId) const {
    return luts[names.indexOf(waveId)];
}

const dsp::LookupTable<float>* LfoTable::operator [](int waveIndex) const {
    return luts[waveIndex];
}

LfoTable::LfoTable() {
    // root directory
    File root{"/Users/mgoheen/JUCE/Projects/cVoices/Resources"};
    
    // add each default wave
    addShapeFromBinary("sin_2048_wav", "sine");
    addShapeFromBinary("tri_2048_wav", "triangle");
    addShapeFromBinary("sqr_2048_wav", "square");
    addShapeFromBinary("saw_2048_wav", "saw");
}

LfoTable::~LfoTable() {}

void LfoTable::addShapeFromFile(File wavFile, String name)
{
    jassert(!names.contains(name));
    
    AudioFormatManager afm;
    afm.registerBasicFormats();
    std::unique_ptr<AudioFormatReader> reader(afm.createReaderFor(wavFile));
    
    readShapeFromAfr(reader.get());
    
    names.add(name);
}

void LfoTable::addShapeFromBinary(const char* resourceName, String name)
{
    jassert(!names.contains(name));
    
    int sz;
    const char* data = BinaryData::getNamedResource(resourceName, sz);
    
    AudioFormatManager afm;
    afm.registerBasicFormats();
    std::unique_ptr<AudioFormatReader>
        reader(afm.createReaderFor(new MemoryInputStream(data, sz, false)));
                                              
    readShapeFromAfr(reader.get());
    
    names.add(name);
}

void LfoTable::readShapeFromAfr(AudioFormatReader* reader) {
    AudioBuffer<float> buffer(1, numPoints+1);
    
    if (reader != nullptr) {
        if(reader->lengthInSamples == numPoints) {
            reader->read(&buffer, 0, numPoints, 0, false, false);
            buffer.setSample(0, numPoints, buffer.getSample(0, 0));
        }
    }
    
    luts.add(new dsp::LookupTable<float>([&buffer] (size_t i) { return buffer.getSample(0, static_cast<int>(i)); }, numPoints+1));
}


Lfo::Lfo(const LfoTable& table, float sr)
  : lfoTable(table),
    sampleRate(sr),
    numPoints(table.numPoints)
{
    setFrequency(1.0f);
    setWaveType("sine");
    
    reset();
}

Lfo::~Lfo() {}

void Lfo::setFrequency(float frequency) {
    increment = frequency * numPoints / sampleRate;
}

void Lfo::setWaveType(String waveId) {
    currentTable = lfoTable[waveId];
}

void Lfo::setWaveType(int waveIndex) {
    currentTable = lfoTable[waveIndex];
}

void Lfo::reset() {
    readIndex = 0.0f;
}

forcedinline float Lfo::getNextSample() {
    // get index to read from by applying phase offset
    float readPos = readIndex + phaseOffset * numPoints;
    if (readPos >= numPoints) readPos -= numPoints;
    
    // get output sample
    float outputSample = currentTable->getUnchecked(readPos);
    
    // increment the read index
    readIndex += increment;
    if (readIndex >= numPoints) readIndex -= numPoints;

    return outputSample;
}

void Lfo::fillBuffer(float* buffer, int numSamples) {
    for (int i = 0; i < numSamples; i++) {
        buffer[i] = getNextSample();
    }
}
