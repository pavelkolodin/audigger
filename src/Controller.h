/*
 *  pavelkolodin@gmail.com
 */

#ifndef _Controller_H_
#define _Controller_H_

#include <vector>
#include "Settings.h"
#include "TracksBase.h"
#include "TracksGroup.h"
#include "Processor.h"



#ifdef AUDIO_VIRT
	class AudioDeviceVirt;
	typedef AudioDeviceVirt AudioDevice_t;
#else
	#ifdef WIN32
		class AudioDeviceWinAPI;
		typedef AudioDeviceWinAPI AudioDevice_t;
	#else
		class AudioDeviceALSA;
		typedef AudioDeviceALSA AudioDevice_t;
	#endif
#endif

class Controller
{		
	public:
		Controller ( );
		virtual ~Controller ( );
		
		Processor::Processor *getProcessor() { return m_proc.get(); }
		Settings::Settings& getSettings() { return m_settings; }
		TracksBase::TracksBase* getBase(const char *_name);

		TracksGroup::Ptr createTracksGroup();
		bool deleteTracksGroup( const TracksGroup* _ptr );

		double getPreciseSampleRate();
		bool sndStart(TracksGroup::Ptr _group);
		void sndStop();

	private:
        Controller ( const Controller& _other );
        Controller& operator= ( const Controller& _other );

        boost::shared_ptr< Processor::Processor > m_proc;
        //boost::shared_ptr< TracksManager > m_tracks_manager;
        boost::shared_ptr< AudioDevice_t > m_sounddevice;
		Settings::Settings m_settings;
		std::vector <TracksGroup::Ptr> m_groups;

		boost::shared_ptr< TracksBase::TracksBase > m_base;
};

#endif

