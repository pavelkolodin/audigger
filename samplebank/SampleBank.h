/*
 * pavelkolodin@gmail.com
 * 2013-11-27 03:51
 */

#ifndef SAMPLEBANK_H_
#define SAMPLEBANK_H_

#include <iostream>
#include "TracksGroup.h"
#include "TrackAudio.h"
#include "TrackMarks.h"

class SampleBank
{
public:
	SampleBank();

	void process(const std::string &_dir_project, const std::string &_dir_output);

};

#endif /* SAMPLEBANK_H_ */
