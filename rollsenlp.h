#ifndef ROLLSENLP_HPP
#define ROLLSENLP_HPP

	/**
	 * Change filter parameters
	 */
	void Lp_set_cutoff_frequency(float sample_freq, float cutoff_freq);

	/**
	 * Add a new raw value to the filter
	 *
	 * @return retrieve the filtered result
	 */
	float LpApply(float sample);

	/**
	 * Reset the filter state to this value
	 */
	float LpReset(float sample);

#endif