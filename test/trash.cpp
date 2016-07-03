/*
 * notepad.cpp
 *
 *  Created on: May 26, 2012
 *      Author: xx
 */

//
// T R A S H
//

	// ---------------------
	// We must set new current tab (via setCurrentIndex()) BEFORE we change "m_curr_project",
	// because "setCurrentIndex" invokes slotProjectCurrentChanged(),
	// that needs to HIDE old current tab.
	//tabWidget->setCurrentIndex( m_projectUI.size() - 1 );
	//m_curr_project = tc;
	// ---------------------



bool TracksGroup::addTrackInfo( const TrackInfo &_track )
{
	//_track.print();
	// Void track?
	if ( TrackInfo::UNDEFINED == _track.m_type )
		return false;

	// No tracks?
	if ( NULL == _track.m_track_audio && NULL == _track.m_track_marks )
		return false;

	std::vector < TrackInfo* >::iterator it, ite;
	it = m_tracks.begin();
	ite = m_tracks.end();
	for ( ; it != ite; ++it )
		if ( ((*it)->m_track_marks == _track.m_track_marks) || ( (*it)->m_track_audio == _track.m_track_audio ) )
			return false;


	if ( TrackInfo::MARKS == _track.m_type )
	{
		m_tracks_marks.push_back( &_track );
	}

	if ( TrackInfo::AUDIO == _track.m_type )
	{
		m_tracks_audio.push_back( &_track );
	}

	// Set samplerate to all marks tracks:
	if ( m_tracks_audio.size() )
	{
		unsigned rate = m_tracks_audio[0]->m_track_audio->getAudioParams().m_rate;
		std::vector < TrackInfo* >::iterator it, ite;
		it = m_tracks_marks.begin();
		ite = m_tracks_marks.end();
		for ( ; it != ite; ++it )
			(*it)->m_track_marks->setSampleRate( rate );

		m_params = m_tracks_audio[0]->m_track_audio->getAudioParams();
	}
	return true;
}

void TracksGroup::delTrack( const TrackInfo* _p )
{
	std::vector < TrackInfo* >::iterator it, ite;
	it = m_tracks_audio.begin();
	ite = m_tracks_audio.end();
	for ( ; it != ite; ++it )
		if ( *it == _p )
		{
			m_tracks_audio.erase( it );
			break;
		}

	it = m_tracks_marks.begin();
	ite = m_tracks_marks.end();
	for ( ; it != ite; ++it )
		if ( *it == _p )
		{
			m_tracks_marks.erase( it );
			break;
		}
}





//	connect ( wtm->pushButton_new, SIGNAL(clicked()), this, SLOT(slotProjectNew()) );
//	connect ( wtm->pushButton_openMedia, SIGNAL(clicked()), this, SLOT(slotProjectOpen()) );
//	connect ( wtm->pushButton_saveMedia, SIGNAL(clicked()), this, SLOT(slotProjectSave()) );
//	connect ( wtm->pushButton_closeMedia, SIGNAL(clicked()), this, SLOT(slotProjectClose()) );
//	connect ( wtm->pushButton_copy, SIGNAL(clicked()), this, SLOT(slotCopy()));
//	connect ( wtm->pushButton_paste, SIGNAL(clicked()), this, SLOT(slotPaste()));
//	connect ( wtm->pushButton_insert, SIGNAL(clicked()), this, SLOT(slotButtonInsert()));
//	connect ( wtm->pushButton_undo, SIGNAL(clicked()), this, SLOT(slotButtonUndo()));
//	connect ( wtm->pushButton_redo, SIGNAL(clicked()), this, SLOT(slotButtonRedo()));
//	connect ( wtm->pushButton_sndStart, SIGNAL(clicked()), this, SLOT(slotSndStart()));
//	connect ( wtm->pushButton_sndPause, SIGNAL(clicked()), this, SLOT(slotSndPause()));
//	connect ( wtm->comboBox_step, SIGNAL(currentIndexChanged(int)), this, SLOT(slotFFTStep(int)));

//	connect ( wtp->m_qbg, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotPaintBar()) );
//	connect ( wtp->m_band, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(slotPaintBar()) );
//	connect ( wtp->pushButton_apply, SIGNAL(clicked()), this, SLOT(slotApply()) );
//	connect ( wtp->doubleSpinBox_param1, SIGNAL(valueChanged(double)), this, SLOT(slotPaintBar()) );



		//QMenuBar *m_menubar;
		//QAction *m_a_mainbar, *m_a_paintbar, *m_a_statusbar;

		//WidgetToolsMain *m_widgettoolsmain;
		//WidgetToolsPaint *m_widgettoolspaint;


//void WindowMain::_buildMenuBar()
//{
//
//	m_menubar = new QMenuBar(this);
//	m_menubar->setFixedHeight( 26 );
//	m_menubar->move(0, 0);
//	QMenu *m_menu_file = m_menubar->addMenu( "File" );
//	QMenu *m_menu_track = m_menubar->addMenu( "Track" );
//	QMenu *m_menu_edit = m_menubar->addMenu( "Edit" );
//	QMenu *m_menu_windows = m_menubar->addMenu( "Windows" );
//	QMenu *m_menu_help = m_menubar->addMenu( "Help" );
//
//	//
//	// File
//	//
//	QAction *a_newproject = m_menu_file->addAction( "New" );
//	a_newproject->setShortcuts( QKeySequence::New );
//	QAction *a_openmedia = m_menu_file->addAction( "Open" );
//	a_openmedia->setShortcuts( QKeySequence::Open );
//	QAction *a_savemedia = m_menu_file->addAction( "Save" );
//	a_savemedia->setShortcuts( QKeySequence::Save );
//	QAction *a_savemediaas = m_menu_file->addAction( "Save As ..." );
//	a_savemediaas->setShortcuts( QKeySequence::SaveAs );
//	QAction *a_closeproject = m_menu_file->addAction( "Close" );
//	a_closeproject->setShortcuts( QKeySequence::Close );
//	m_menu_file->addSeparator();
//	QAction *a_quit = m_menu_file->addAction( "Quit" );
//	a_quit->setShortcuts( QKeySequence::Quit );
//
//	connect ( a_newproject, SIGNAL(triggered(bool)), this, SLOT(slotProjectNew()) );
//	connect ( a_openmedia, SIGNAL(triggered(bool)), this, SLOT(slotProjectOpen()) );
//	connect ( a_savemediaas, SIGNAL(triggered(bool)), this, SLOT(slotProjectSave()) );
//	connect ( a_closeproject, SIGNAL(triggered(bool)), this, SLOT(slotProjectClose()) );
//	connect ( a_quit, SIGNAL(triggered(bool)), this, SLOT(close()) );
//
//	//
//	// Track
//	//
//	QAction *a_trackaudionew = m_menu_track->addAction( "New Audio" );
//	QAction *a_trackaudioopen = m_menu_track->addAction( "Open Audio" );
//	QAction *a_trackmarksnew = m_menu_track->addAction( "New Marks" );
//	QAction *a_trackmarksopen = m_menu_track->addAction( "Open Marks" );
//
//	connect ( a_trackaudionew, SIGNAL(triggered(bool)), this, SLOT(slotTrackAudioNew()) );
//	connect ( a_trackaudioopen, SIGNAL(triggered(bool)), this, SLOT(slotTrackAudioOpen()) );
//	connect ( a_trackmarksnew, SIGNAL(triggered(bool)), this, SLOT(slotTrackMarksNew()) );
//	connect ( a_trackmarksopen, SIGNAL(triggered(bool)), this, SLOT(slotTrackMarksOpen()) );
//
//	//
//	// Edit
//	//
//	QAction *a_undo = m_menu_edit->addAction( "Undo" );
//	a_undo->setShortcuts( QKeySequence::Undo );
//	QAction *a_redo = m_menu_edit->addAction( "Redo" );
//	//a_redo->setShortcuts( QKeySequence::Redo ); // i don't like CTRL+SHIFT+Z
//	a_redo->setShortcut( QKeySequence( Qt::CTRL | Qt::Key_Y) );
//	m_menu_edit->addSeparator();
//	QAction *a_cut = m_menu_edit->addAction( "Cut" );
//	a_cut->setShortcuts( QKeySequence::Cut );
//	QAction *a_copy = m_menu_edit->addAction( "Copy" );
//	a_copy->setShortcuts( QKeySequence::Copy );
//	QAction *a_paste = m_menu_edit->addAction( "Paste" );
//	a_paste->setShortcuts( QKeySequence::Paste );
//
//	connect ( a_undo, SIGNAL(triggered(bool)), this, SLOT(slotButtonUndo()));
//	connect ( a_redo, SIGNAL(triggered(bool)), this, SLOT(slotButtonRedo()));
//	connect ( a_cut, SIGNAL(triggered(bool)), this, SLOT(slotCut()));
//	connect ( a_copy, SIGNAL(triggered(bool)), this, SLOT(slotCopy()));
//	connect ( a_paste, SIGNAL(triggered(bool)), this, SLOT(slotPaste()));
//
//	//connect ( , SIGNAL(triggered(bool)), this, SLOT());
//
//	m_a_mainbar = m_menu_windows->addAction( "Toolbar" );
//	m_a_mainbar->setCheckable( true );
//	m_a_mainbar->setChecked( true );
//	m_a_paintbar = m_menu_windows->addAction( "Paintbar" );
//	m_a_paintbar->setCheckable( true );
//	m_a_paintbar->setChecked( false );
//	m_a_statusbar = m_menu_windows->addAction( "Statusbar" );
//	m_a_statusbar->setCheckable( true );
//	m_a_statusbar->setChecked( true );
//
//	connect (m_a_mainbar, SIGNAL(triggered(bool)), this, SLOT(slotBars()) );
//	connect (m_a_paintbar, SIGNAL(triggered(bool)), this, SLOT(slotBars()) );
//	connect (m_a_statusbar, SIGNAL(triggered(bool)), this, SLOT(slotBars()) );
//
//
//	QAction *a_help = m_menu_help->addAction( "About" );
//	connect ( a_help, SIGNAL(triggered(bool)), this, SLOT( slotInfo()) );
//}



//// !!! DEPRFUNC
//void WindowMain::slotPaintBar()
//{
//	int lim = m_tcs.size();
//	for ( int i = 0; i < lim; ++i )
//	{
//
//		size_t tracks = m_tcs[i]->numTracks();
//		for ( size_t t = 0; t < tracks; ++t )
//		{
//			IWidgetTrack::MouseMode mode = IWidgetTrack::MM_DRAG;
//
//			if ( m_widgettoolspaint->pushButton_drag->isChecked() )
//				mode = IWidgetTrack::MM_DRAG;
//			else
//			if ( m_widgettoolspaint->pushButton_select_time->isChecked() )
//				mode = IWidgetTrack::MM_SELECTTIME;
//			else
//			if ( m_widgettoolspaint->pushButton_select_both->isChecked() )
//				mode = IWidgetTrack::MM_SELECTBOTH;
//			else
//			if ( m_widgettoolspaint->pushButton_pen->isChecked() )
//				mode = IWidgetTrack::MM_PEN;
//			else
//			if ( m_widgettoolspaint->pushButton_eraser->isChecked() )
//				mode = IWidgetTrack::MM_ERASER;
//
//			m_tcs[i]->getTrack(t)->setMouseMode( mode );
//		}
//
//		// DSP mode
//		if ( m_widgettoolspaint->pushButton_band_pass->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeBandpass(); //getDSP()->setMode( DSP::BANDPASS );
//		else
//		if ( m_widgettoolspaint->pushButton_band_pass_zero->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeBandpassZero(); //getDSP()->setMode( DSP::BANDPASS_ZERO );
//		else
//		if ( m_widgettoolspaint->pushButton_band_suppress->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeSuppress(); //getDSP()->setMode( DSP::BANDSUPPRESS );
//		else
//		if ( m_widgettoolspaint->pushButton_F1->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeF1(); //getDSP()->setMode( DSP::BANDSUPPRESS );
//		else
//		if ( m_widgettoolspaint->pushButton_F2->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeF2(); //getDSP()->setMode( DSP::BANDSUPPRESS );
//		else
//		if ( m_widgettoolspaint->pushButton_F3->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeF3(); //getDSP()->setMode( DSP::BANDSUPPRESS );
//		else
//		if ( m_widgettoolspaint->pushButton_F4->isChecked() )
//			m_ctrl->getProcessor()->setDSPModeF4(); //getDSP()->setMode( DSP::BANDSUPPRESS );
//
//
//
//		m_ctrl->getProcessor()->setDSPParam1(m_widgettoolspaint->doubleSpinBox_param1->value() );
//	}
//
//	//m_curr_trackscontainer->setPenEnabled( pen_enabled );
//}



	// ===
	// We must not call m_ctrl->loadData() JUST AFTER SWITCHING,
	// because windows is still playing LAST PERIOD of previous SLOT
	// while we switching to new slot and re-writing previous SLOT,
	// and there is possibility to re-write currently playing slot.
	// ===




	// Examine audio devices.
	// AUX
	std::fstream f1;
	f1.open( "audio.txt", std::ios_base::out );
	int dnum = auxGetNumDevs();
	f1 << "AUX dnum " << dnum << "\n";
	MMRESULT res;
	AUXCAPS auxcaps;
	WAVEINCAPS waveincaps;
	WAVEOUTCAPS waveoutcaps;
	for ( unsigned i = 0; i < dnum; ++i )
	{
		res = auxGetDevCaps(i, &auxcaps, sizeof(AUXCAPS));
		f1 << "    dev " << i << ": " << auxcaps.szPname << "\n";
	}

	// In
	dnum = waveInGetNumDevs();
	f1 << "IN dnum " << dnum << "\n";
	for ( unsigned i = 0; i < dnum; ++i )
	{
		res = waveInGetDevCaps(i, &waveincaps, sizeof(WAVEINCAPS));
		f1 << "    dev " << i << ": " << waveincaps.szPname << "\n";
	}

	// Out
	dnum = waveOutGetNumDevs();
	f1 << "OUT dnum " << dnum << "\n";
	for ( unsigned i = 0; i < dnum; ++i )
	{
		res = waveOutGetDevCaps(i, &waveoutcaps, sizeof(WAVEOUTCAPS));
		f1 << "    dev " << i << ": " << waveoutcaps.szPname << "\n";
	}

	f1.close();





	// Rate measurements.
	// (windows)
	if ( m_cnt_play == 16 )
		m_time_start = getCurrentTimeMs();

	if ( m_cnt_play > 16 )
	{
		m_rate_mean_datasize += currsize;

		long long currtime = getCurrentTimeMs();
		double dt = (double)(currtime - m_time_start);
		// DATA SENT
		m_lastsentperiod_time = currtime;
		m_lastsentperiod_frame = m_frames_sent;
		m_frames_sent += currsize;

		if ( dt > 5000.0 )
		{
			MMTIME mmtime;
			mmtime.wType = TIME_SAMPLES;
			//res = waveOutGetPosition(m_handler_winapi, &mmtime, sizeof(MMTIME) );
			//if ( MMSYSERR_NOERROR == res )
			{
				// Lets correct measurements.
				m_rate_mean += ((double) m_rate_mean_datasize) * (1000.0 / dt );
				m_rate_mean /= 2;
				m_rate_mean_datasize = 0;
				m_time_start = currtime;
				LOGVAR1 ( m_rate_mean );
			}
		}

	} // m_cnt_play > 16






		//
		// Precise rate measurement:
		// (linux)

		// 64 calls of play() is enough to reach "stable mode". Wtf? Why?
		if ( m_cnt_play == 64 ) // TODO magic 64
			m_time_start = getCurrentTimeMs();

		if ( m_cnt_play > 64 )
		{
			m_rate_mean_datasize += currsize;
			long long currtime = getCurrentTimeMs();
			double dt = currtime - m_time_start;
			if ( dt > 5000.0 )	// TODO magic 5000.0
			{
				// Lets correct measurements.
				m_rate_mean += ((double) m_rate_mean_datasize) * (1000.0 / dt );
				m_rate_mean /= 2;
				m_rate_mean_datasize = 0;
				m_time_start = currtime;
				LOGCOUT ( "\n(RATE " << m_rate_mean << ")" )
			}
		}




////////////////////////pass



	// curv_left begin:
	int cl_beg = (_start < curv_size)?0:_start - curv_size;

	// Left side:
	for ( i = 0; i < cl_beg; ++i )
		_p[i] = 0;

	//curv_left:
	c = (_start < curv_size)?(curv_size - _start):0;
	i = cl_beg;
	for ( ; c < curv_size; ++c, ++i )
		_p[i] *= curv[c];

	// curv_right:
	int ilim = (_start + _len + curv_size <= _win/2 )?(_start+_len+curv_size):_win/2;
	for ( c = curv_size-1, i = _start + _len; i <= ilim; ++i, --c )
		_p[i] *= curv[c];

	// Right side:
	for ( i = _start + _len + curv_size; i <= _win/2; ++i )
		_p[i] = 0;



	// Mirror:
	if ( _win/2 == _len + _start) // not probably
		-- _len;

	if ( 0 == _start )
	{
		++_start;
		--_len;
	}

	// Right side:
	for ( i = _win/2 + 1; i < _win - (_start + _len + curv_size); ++i )
		_p[i] = 0;

	// right curve:
	if (_win - (_start + _len + curv_size) > _win/2)
	{
		i = _win - (_start + _len + curv_size);

	}
	else
		i = (_win/2)+1;
	ilim =
	c = (i - (_win/2 + 1) >= curv_size)?0:curv_size - (i - (_win/2 + 1));

	for ( ; c < lim; ++i, ++c)
		_p[i] *= curv[c];

////////////////////////

void spectBandSuppress(complex *_p, size_t _win, size_t _start, size_t _len, float _suppress)
{
	static const size_t curv_size = 4;
	static const float curv[] = {
					0.9510565162951535,
					0.8090169943749475,
					0.5877852522924731,
					0.30901699437494745
	};

	/*
	static const size_t curv_size = 7;
	static const float curv[] = {
			0.9807852804032304,
			0.9238795325112867,
			0.8314696123025452,
			0.7071067811865476,
			0.5555702330196023,
			0.38268343236508984,
			0.19509032201612833,
	};

	*/
	/*
	static const size_t curv_size = 2;
	static const float curv[] = {
			0.8660254037844387,
			0.5000000000000001
	};
	*/

	if ( 0 == _len || _start + _len > _win/2 )
		return;


	size_t curv_left = curv_size, curv_right = curv_size;
	if ( _len < curv_size*2 )
	{
		curv_left = _len/2;
		curv_right = _len - curv_left;
	}

	if ( _start < curv_left )
	{
		curv_left = 0;
		_start = 0;
	}
	if ( _start + _len >= (_win/2) - curv_right )
	{
		curv_right = 0;
		_len = _win/2 - _start;
	}

	for ( int i = _start, c = 0; i < _start + curv_left; ++i, ++c )
		_p[i] *= curv[c];
	for ( int i = _start + curv_left; i < _start+_len - curv_right; ++i )
		_p[i] = 0;

	// (_start+_len+1), "+1" - because _win/2 is HIGHEST FREQUENCY
	for ( int i = (_start+_len+1) - curv_right, c = curv_size-1; i <= _start+_len; ++i, --c)
		_p[i] *= curv[c];

	// Mirror

	if ( _win/2 == _len + _start)
		-- _len;

	if ( 0 == _start )
	{
		++_start;
		--_len;
	}

	for ( int i = _win - (_start + _len), c = 0; i < _win - (_start + _len - curv_right); ++i, ++c)
		_p[i] *= curv[c];

	for ( int i = (_win-1) - (_start + _len - curv_right); i < _win - (_start + curv_left); ++i )
		_p[i] = 0;

	for ( int i = (_win-1) - (_start + curv_left), c = curv_size-1; i < _win - _start; ++i, --c)
		_p[i] *= curv[c];


	//for ( int i = _win - _start; i > _win - (_start + _len); --i)
	//	_p[i] *= _suppress;
}








//	// Convert LLLLLRRRRR to LRLRLRLRLR:
//	for (unsigned i = 0; i < m_fft_win; ++i )
//	{
//		//win[channels*i] = win_x[i].re();
//		double r = fir::intRound<double>( m_complex[i].re() );
//		//double r = m_complex[i].re();
//		if ( r > 32767 )
//			r = 32767;
//		if ( r < -32767 )
//			r = -32767;
//		m_face[m_channels*i] = r;
//	}
//
//	if (2 == m_channels)
//		for (unsigned i = 0; i < m_fft_win; ++i )
//			{
//			//win[channels*i + 1] = win_x[_res + i].re();
//				double r =
//				//double r =  m_complex[m_fft_win + i].re();
//				if ( r > 32767 )
//					r = 32767;
//				if ( r < -32767 )
//					r = -32767;
//
//				m_face[m_channels*i + 1] = limitShort( fir::intRound<double>( m_complex[m_fft_win + i].re() ) );
//			}








//	/// Spectrum changing task.
//	void filter( unsigned _track_id, unsigned _res, unsigned _step, size_t _from, float *_values, size_t _steps );
//
//	/// Spectrum changing task.
//	void draw( unsigned _track_id, unsigned _res, unsigned _step, size_t _from, float *_values, size_t _steps );
//
//	void test();


// _ftr - array of float, size = (_res/2) * _lines
// [_res/2][_res/2][_res/2].... = [freqs][freqs][freqs]...
// Contains user's pencil paintings.
void Processor::filter( unsigned _track_id, unsigned _res, unsigned _step, size_t _from, float *_ftr, size_t _lines )
{
	if ( m_tracks.size() <= _track_id )
		return;
	if ( ! m_tracks[ _track_id ] )
		return;
	if ( _lines < 1 )
		return;

	if ( _step > _res/2 )
		return;

	unsigned channels = m_tracks[ _track_id ]->getAudioParams().m_channels;

	size_t sz = _res + (_step * (_lines-1));
	// Read samples from buffer:
	short *smp = new short[ sz * channels ];
	short *smp_out = new short[ sz * channels ];
	memset( smp, 0, sizeof(short) * sz * channels);
	memset( smp_out, 0, sizeof(short) * sz * channels);

	short *win = new short[ _res*channels ];
	complex *win_x = new complex [ _res*channels ];

	//
	// Read
	//
	size_t ret_read = m_tracks[ _track_id ]->read(smp, _from, sz );

	LOGVAR1( ret_read )
	LOGVAR4( _track_id, _res, _step, _from )
	LOGVAR1( _lines )

	// Process with N/2 step.
	short *smp_p = smp;
	short *smp_out_p = smp_out;
	short *end = smp + ret_read*channels;
	float *ftr = _ftr;

	// Step in filtering data.
	size_t step_mul_fdata = (_res/2)/_step;

	for ( ; smp_p + _res*channels <= end;
			smp_p += ((_res/2)*channels),
			smp_out_p += ((_res/2)*channels),
			ftr += (_res/2)*step_mul_fdata)
	{
		//LOGPLACE
		memcpy( win, smp_p, sizeof(short)*_res*channels );

		// hann window
		hann( win, _res, channels );

		// Convert LRLRLRLRLR to LLLLLRRRRR
		for ( unsigned i = 0; i < _res; ++i )
			win_x[i] = win[channels * i];

		if ( channels > 1)
			for ( unsigned i = 0; i < _res; ++i )
				win_x[_res + i] = win[channels * i + 1];

		// Channel 1
		CFFT::Forward(win_x, _res);
				// Channel 2
		if (channels > 1)
			CFFT::Forward(win_x + _res, _res);

		// Manipulate spectrum:
//		for ( int i = 50; i < 90; ++i )
//		{
//			win_x[i] = 0; // Re + im.
//			win_x[(_res-1) - i] = 0; // Re + im.
//			if (channels > 1)
//			{
//				win_x[_res +  i] = 0;
//				win_x[_res +  ((_res-1) - i)] = 0;
//			}
//		}

		//_spectModify( complex *_spec, size_t _size, SpectOpCode _opcode, float *_modifier);
		_spectModify( win_x, _res, FILTER, ftr);
		if ( 2 == channels )
			_spectModify( win_x + _res, _res, FILTER, ftr);

		// Back FFT.
		CFFT::Inverse(win_x, _res);
		if (channels > 1)
			CFFT::Inverse(win_x + _res, _res);

		// Convert LLRR to LRLR:
		for (unsigned i = 0; i < _res; ++i )
			//win[channels*i] = win_x[i].re();
			win[channels*i] = fir::intRound<double>( win_x[i].re() );
		if (channels > 1)
			for (unsigned i = 0; i < _res; ++i )
				//win[channels*i + 1] = win_x[_res + i].re();
				win[channels*i + 1] = fir::intRound<double>( win_x[_res + i].re() );

		// Sum:
		sumSamples( smp_out_p, win, _res, channels );
	}


	if ( smp_p != smp )
	{
		// Apply RIGHT PART of hann window at begin
		// Apply LEFT PART of hann window at end
		// BEGIN
		memcpy( win, smp, sizeof(short)*(_res/2)*channels );
		hannRight( win, _res/2, channels );
		sumSamples( smp_out, win, _res/2, channels );

		// END
		// get last 0.5 win.
		//smp_p -= ((_res/4)*channels);
		//smp_out_p -= ((_res/4)*channels);

		memcpy( win, smp_p, sizeof(short)*(_res/2)*channels );
		hannLeft( win, _res/2, channels );

		sumSamples( smp_out_p, win, _res/2, channels );


		// Add not processed samples:
		short *end_written = smp_out_p + (_res/2)*channels;
		short *end_read = smp_p + (_res/2)*channels;

		size_t need_frames = ret_read - (end_written - smp_out)/channels;
		LOGCOUT("need frames: " << need_frames );
		memcpy ( end_written, end_read, sizeof(short) * need_frames * channels );

		// OVERWRITE piece of signal in track.
		// false: overwrite, true: insert.
		m_tracks[ _track_id ]->insertData( _from, smp_out, ret_read, false );
	}
	else
		delete[] smp_out;

	delete[] smp;
	delete[] win;
	delete[] win_x;
	// Don't delete "smp_out", it is inserted into track!


//	unsigned _newtrack = 0;
//	createTrack( &_newtrack );
//	m_tracks[ _newtrack ]->insertData( 0, win, _res, true );
//	exportTrack( _newtrack, "/tmp/zzz.wav" );
}

/// Spectrum changing task.
void Processor::draw( unsigned _track_id, unsigned _res, unsigned _step, size_t _from, float *_ftr, size_t _steps )
{
	LOGVAR5( _track_id, _res, _step, _from, _steps );

	if ( m_tracks.size() <= _track_id )
		return;
	if ( ! m_tracks[ _track_id ] )
		return;
	if ( _steps < 1 )
		return;

	LOGPLACE
	if ( _step != _res && 2*_step != _res && 4*_step != _res && 8*_step != _res && 16*_step != _res)
		return;
	LOGPLACE
	if ( ! goodFFTWin( _res) )
		return;
	LOGPLACE
	float win_coeff = ((float)_step)/(float)_res;
	LOGVAR1 (win_coeff);

	AudioTrack *tr = m_tracks[ _track_id ];
	unsigned channels = tr->getAudioParams().m_channels;
	size_t frames = _res + _step * (_steps - 1);
	LOGVAR1 ( frames )

	short *smp = NULL, *win = NULL;
	complex *win_x = NULL;
	try{
		smp = new short[ frames * channels ];
		win = new short[ _res*channels ];
		win_x = new complex [ _res*channels ];

	} catch ( ... )
	{
		if ( smp ) delete[] smp;
		if ( win ) delete[] win;
		if ( win_x ) delete[] win_x;
		return;
	}
	LOGPLACE

	size_t ret_read = tr->read(smp, _from, frames );

	//memset( smp, 0, sizeof(short) * frames*channels );
	//memset( win, 0, sizeof(float) * _res*channels );

	short *smp_p = smp;
	float *ftr_p = _ftr;
	LOGPLACE
	for ( int i = 0; i < _steps; ++i, smp_p += _step * channels, ftr_p += _res/2 )
	{
		memset( win_x, 0, sizeof(complex) * _res * channels );

		spectAddArray( win_x, _res, ftr_p );
		if (channels > 1)
			spectAddArray( win_x + _res, _res, ftr_p );

		// Back FFT.
		CFFT::Inverse(win_x, _res);
		if (channels > 1)
			CFFT::Inverse(win_x + _res, _res);

		spectToFrames(win, win_x, _res, channels );

		hannWeight(win, _res, channels, win_coeff );
		sumSamples(smp_p, win, _res, channels );
	}

	tr->insertData( _from, smp, frames, false );
}




void Processor::test()
{
	size_t winsize = 2048;
	short *win = new short[ winsize ];
	complex *win_x = new complex[ winsize ];

	for ( unsigned i = 0; i < winsize; ++i)
	{
		win[i] = (short)(20000.0 * sin ( 64*(M_PI*2*((float)i) ) / (float)(winsize - 1) ) );
		win_x[i] = win[i];
	}

	CFFT::Forward(win_x, winsize);

	std::fstream f1 ( "spect.txt", std::ios_base::out | std::ios_base::binary );
	if ( ! f1.good() )
		return;

	for ( unsigned i = 0; i < winsize; ++i)
	{
		f1 << i << " " << win_x[i].re() << " " << win_x[i].im() << "\n";
	}

	f1.close();
}




void Processor::_spectModify( complex *_spec, size_t _size, SpectOpCode _opcode, float *_modifier)
{
	// Don't touch freq = 0 :)

	spectShift( _spec, _size, 32);
	return;

	if ( ::Processor::FILTER == _opcode )
	{
		for ( int f = 1; f < _size/2; ++f )
		{
			if ( 0 != _modifier[f])
			{
				// Apply pen's data.
				size_t start = f;
				size_t band = 1;
				spectIHann( &_spec[start], band);
				spectIHann( &_spec[_size - (start+band)], band);
			}
		}
		return;
	}
	else
	if ( ::Processor::DRAW == _opcode )
	{
		for ( int f = 1; f < _size/2; ++f )
		{
			if ( 0 != _modifier[f])
			{
				// Apply pen's data.
				//size_t start = f;
				//size_t band = 3;
				//hannSpectAddValue( &_spec[start], band, 100000.0 );
				//hannSpectAddValue( &_spec[_size - (start+band)], band, 100000.0);

				spectAddValue(_spec, _size, f, 300000.0);

				//_spec[start] += 100000.0;
				//_spec[_size - (start+band)] += 100000.0;

				//hannInverSpect( &_spec[start], band);
				//hannInverSpect( &_spec[_size - (start+band)], band);

				//if (channels > 1)
				//{
				//	hannInverSpect( &_spec[_size + start], band);
				//	hannInverSpect( &_spec[_size + (_size - (start+band))], band);
				//}
			}
		}
		return;
	}

	// stats
	//double max[8];
	//memset(max, 0, sizeof(double));
	//double avg = 0;
	//for ( int i = 0; i < _size/2; ++i )
	//{
	//	if ( _spec[i].re() > max[0] )
	//	{
	//		for ( int i = 7; i > 0; --i )
	//			max[i] = max[i-1];
	//		max[0] = _spec[i].re();
	//	}
	//	avg += _spec[i].re();
	//	avg /= 2;
	//}

	//for ( int i = 0; i < 8; ++i )
	//	LOGCOUT ( " " << max[i] )
}







	// _spectModify(), single channel function.
	// _size = full size of spectrum data (for window 2048, spect size is the same (mirrored included))
	// _opcode = what to do with spectrum?
	// _modifier - array of size "_size".
	void _spectModify( complex *_spec, size_t _size, SpectOpCode _opcode, float *_modifier);


/*


		// Don't touch freq = 0 :)
		for ( int f = 1; f < _res/2; ++f )
		{
			if ( 0 != ftr[f])
			{
				// Apply pen's data.
				size_t start = f;
				size_t band = 10;
				hannInverSpect( &win_x[start], band);
				hannInverSpect( &win_x[_res - (start+band)], band);
				if (channels > 1)
				{
					hannInverSpect( &win_x[_res + start], band);
					hannInverSpect( &win_x[_res + (_res - (start+band))], band);
				}
			}
		}


*/


// ------------------------------------------------------------------

//
// Reading selection.
//

//	else
//	{
//		short *curr_buff = _dst;
//		size_t curr_size = _len;
//		size_t frames_read = 0;
//
//		// m_selection_from
//		size_t actual_from = _offset;
//		size_t actual_size = 0;
//		if ( _offset > m_selection_from + m_selection_len )
//		{
//			size_t times = (_offset - m_selection_from)/m_selection_len;
//			actual_from -= m_selection_len * times;
//			//actual_size = m_selection_len - (actual_from - m_selection_from);

//		}
//
//		/*
//		if ( task.m_load_from >= m_selection_from + m_selection_len )
//		{
//			size_t times = (int)((task.m_load_from - m_selection_from)/m_selection_len);
//			size_t tail = m_selection_from + task.m_load_from - m_selection_len * times;
//		}
//		 */
//		while ( curr_size > 0 )
//		{
//			// How many frames we can read at a time?
//
//			size_t frames_left = std::min<int>( (m_selection_from + m_selection_len) - actual_from, m_selection_len);
//
//			size_t todo = std::min<int> ( curr_size, frames_left );
//			curr_size -= todo;

//			//usleep(100000);
//
////			sf_count_t sk = sf_seek(m_snd_file, actual_from, SEEK_SET);
////			sf_count_t rd = sf_readf_short(m_snd_file, curr_buff, todo);
//			size_t rd = m_file->read( curr_buff, actual_from, todo );
//
//			curr_buff += rd * m_file->getAudioParams().m_channels; //m_media_file_info.m_channels;
//			frames_read += rd;
//			if ( todo != rd )
//				break;
//
//			actual_from += rd;
//			if ( actual_from >= m_selection_from + m_selection_len)
//				actual_from = m_selection_from;
//
////			actual_size = m_selection_len;
//		}

//		//!//m_ctrl->doneLoadData( frames_read );
//	}

// -------------------------------------------------------------------







// ------------------------------------------------------------------

//void generateAAA(complex *_spect, short *_outdata, unsigned _shift)
//{
//	for ( int i = 0; i < rand()%15 + 1; ++i )
//	{
//		//float df = 200;
//		//unsigned freq = (100 + df*(rand()%20) ) *(65536.0/44100.0);
//		unsigned freq = (20 + (rand()%5000) ) *(65536.0/44100.0);
//
//
//
//		_spect[ freq ] = complex(50*1000000*(1000.0/(float)freq)*(-1*rand()%2), 0); //(-1*rand()%2)*rand()%10000000); //*(rand()%10);
//
//		//_spect[ freq ].i = rand()%10000;
//		//_spect[ freq + 10] = 30*1000000; //*(rand()%10);
//		//_spect[ freq - 10] = 30*1000000; //*(rand()%10);
//		//spect[(int)(10000.0*(65536.0/44100.0)) ] = 15000000;
//	}
//
//	//for ( int j = 0; j < buffsize/2; ++j ) { spect[ (buffsize-1) - j ] = spect[j]; }
//
//
//	for ( int i = 0; i < 44100; ++i )
//	{
//		_outdata[i] = _spect[i].re();
//	}
//	_outdata[0] = 0;
//	_outdata[1] = 0;
//}
//
//void mixHalfOver(short *_dst, const short *_src, size_t _size_in)
//{
//	size_t half_len = _size_in / 2;
//
//	// mix
//	for ( unsigned i = half_len; i < _size_in; ++i)
//	{
//		_dst[i] += _src[i-half_len];
//	}
//
//	for ( unsigned i = half_len; i < _size_in; ++i)
//	{
//		_dst[half_len + i] = _src[i];
//	}
//}

//void Templater::test()
//{
//	SF_INFO		snd_info;
//	snd_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//	snd_info.samplerate = 44100;
//	snd_info.channels = 1;
//
//	SNDFILE		*snd_file = NULL;
//
//	snd_file = sf_open( "test.wav", SFM_WRITE, &snd_info );
//
//	complex *spect = new complex[ 65536 ]; // stereo LLLLRRRR
//	short *outdata = new short[ 65536 ];
//
//	size_t portion = 44100/64;
//
//	short *buff = new short[ portion ];
//	short *mixed = new short[ portion*2 ];
//
//	for ( size_t i = 0; i < portion*2; ++i)
//		mixed[i] = 0;
//
//	for ( int i = 0; i < 120; ++i )
//	{
//		for ( int j = 0; j < portion; ++j )
//			mixed[j] = mixed[portion/2 + j];
//
//		generateAAA( spect, outdata, i );
//		hanning( outdata, portion, 1);
//		for ( int j = 0; j < portion; ++j )
//			buff[j] = outdata[j];
//
//		mixHalfOver( mixed, buff, portion );
//
//		sf_write_short( snd_file, mixed, portion/2 ); // part of a second sec
//	}
//
//	sf_write_sync( snd_file );
//	sf_close( snd_file );
//}





//		QMessageBox mb(QMessageBox::Critical, "Error", "Cannot load audio file.", QMessageBox::Close, this);
//		mb.exec();






	QMenuBar *mb = new QMenuBar(this);
	//QMenu *m1 = new QMenu("File", mb);
	//QMenu *m2 = new QMenu("Edit", mb);

	mb->addMenu("File");
	mb->addMenu("Edit");
	mb->addMenu("FFT");

	mb->setFixedWidth( 400 );
	mb->move( 0, 21 );
	//mb->resize();
	mb->setVisible(true);
	mb->show();

	widget_toolbar->move(0, 42);


	//
	// Menu View
	//
//	QActionGroup *bandsGroup = new QActionGroup(this);
//	bandsGroup->addAction( action256 );
//	bandsGroup->addAction( action512 );
//	bandsGroup->addAction( action1024 );
//	bandsGroup->addAction( action2048 );
//	bandsGroup->addAction( action4096 );
//	bandsGroup->addAction( action8192 );
//	bandsGroup->addAction( action16384 );
//	bandsGroup->addAction( action32768 );
//
//	QActionGroup *stepGroup = new QActionGroup(this);
//	stepGroup->addAction( action1_1 );
//	stepGroup->addAction( action1_2 );
//	stepGroup->addAction( action1_4 );
//	stepGroup->addAction( action1_8 );
//	stepGroup->addAction( action1_16 );
//	stepGroup->addAction( action1_32 );



	//	QPushButton *m_button_add = new QPushButton( this );
	//	m_button_add->setText( "S+" );
	//	m_button_add->resize( 40, 24 );
	//	m_button_add->move(pushButton_info->pos().x() + 30, pushButton_info->pos().y() );


		//connect (pushButton_drag, SIGNAL(toggled(bool)), m_widget_sound, SLOT(dragToggled(bool)));



	/////////////////////////////////////////////


	m_label_position = new QLabel();
	m_label_range = new QLabel();
	m_label_range_len = new QLabel();
	m_label_freq = new QLabel();
	LOGPLACE

	QFontMetrics fm( m_label_position->font() );
	QRect r = fm.boundingRect("0:00:00:00.000");
	m_label_position->setFixedWidth( r.width() );
	m_label_position->setText("0:00:00:00.000");
	m_label_range_len->setFixedWidth( r.width() );
	m_label_range_len->setText("no selection");

	r = fm.boundingRect("00000 Hz");
	m_label_freq->setFixedWidth( r.width() );
	m_label_freq->setAlignment(Qt::AlignRight);

	LOGPLACE

	r = fm.boundingRect("0:00:00:00.000 - 0:00:00:00.000");
	m_label_range->setFixedWidth( r.width() );
	m_label_range->setText("no selection");

//	m_statusbar = this->statusBar();
//	m_statusbar->insertPermanentWidget( 0, m_label_position );
//	m_statusbar->insertPermanentWidget( 1, m_label_range );
//	m_statusbar->insertPermanentWidget( 2, m_label_range_len );
//	m_statusbar->insertPermanentWidget( 3, m_label_freq );

	//m_scrollbar = new QScrollBar(Qt::Vertical, centralWidget());

