#include "rollsenlp.h"

#include "math.h"

#ifndef M_PI_F
#define M_PI_F 3.14159f
#endif

float           _cutoff_freq;
float           _a1 = 0.0;
float           _a2 = 0.0;
float           _b0 = 0.0;
float           _b1 = 0.0;
float           _b2 = 0.0;
float           _delay_element_1 = 0.0;        // buffered sample -1
float           _delay_element_2 = 0.0;        // buffered sample -2

void Lp_set_cutoff_frequency(float sample_freq, float cutoff_freq)
{
	_cutoff_freq = cutoff_freq;

	if (_cutoff_freq <= 0.0f) {
		// no filtering
		return;
	}

	float fr = sample_freq / _cutoff_freq;
	float ohm = tanf(M_PI_F / fr);
	float c = 1.0f + 2.0f * cosf(M_PI_F / 4.0f) * ohm + ohm * ohm;
	_b0 = ohm * ohm / c;
	_b1 = 2.0f * _b0;
	_b2 = _b0;
	_a1 = 2.0f * (ohm * ohm - 1.0f) / c;
	_a2 = (1.0f - 2.0f * cosf(M_PI_F / 4.0f) * ohm + ohm * ohm) / c;
}

float LpApply(float sample)
{
	if (_cutoff_freq <= 0.0f) {
		// no filtering
		return sample;
	}

	// do the filtering
	float delay_element_0 = sample - _delay_element_1 * _a1 - _delay_element_2 * _a2;

	if (!isfinite(delay_element_0)) {
		// don't allow bad values to propagate via the filter
		delay_element_0 = sample;
	}

	float output = delay_element_0 * _b0 + _delay_element_1 * _b1 + _delay_element_2 * _b2;

	_delay_element_2 = _delay_element_1;
	_delay_element_1 = delay_element_0;

	// return the value.  Should be no need to check limits
	return output;
}

float LpReset(float sample)
{
	float dval = sample / (_b0 + _b1 + _b2);
	_delay_element_1 = dval;
	_delay_element_2 = dval;
	return apply(sample);
}