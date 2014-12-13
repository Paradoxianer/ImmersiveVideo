/*
  -----------------------------------------------------------------------------

	File: MediaEffect.h
	
	Date: Thursday November 10, 2005

	Description: Media Effect Header
	

	Copyright 2005, yellowTAB GmbH, All rights reserved.
	

  -----------------------------------------------------------------------------
*/

#ifndef _MEDIA_FILTER_H
#define _MEDIA_FILTER_H

#include <support/String.h>
#include <MediaDefs15.h>

class BBuffer;
class BMediaEffect;
class BParameterWeb;

extern "C" BMediaEffect* make_media_effect(void);

class BMediaEffect 
{
//	protected: //seems to have problems
	public:
		/* this has to be at the top to force a vtable */
		virtual					~BMediaEffect();

	public:
		explicit 				BMediaEffect( const BString& _name 
									, const BString& _description
									, const BString& _copyright
									, const MEDIA_KIT_15::node_kind& _kind );
		virtual status_t		HandleMessage(
									int32 		_message,
									const void* _data,
									size_t		_size); //obsolete???

		
		// BControllable methods
		virtual status_t		GetParameterWeb( BParameterWeb** );
		
		virtual status_t 		GetParameterValue(
									int32 		_id,
									bigtime_t* 	_lastChange,
									void* 		_value,
									size_t* 	_ioSize);

		virtual void 			SetParameterValue(
									int32 		_id,
									bigtime_t 	_when,
									const void* _value,
									size_t 		_size);
									
		// Effect Handling Hooks
		virtual status_t		PrepareToRun( void );
		virtual status_t		HandleBuffer( BBuffer* _inBuffer
											, BBuffer* _outBuffer ) = 0;
		
		media_format			GetFormat( void );
		const BString&	Copyright(void);
		const BString&	GetName(void);
		const BString&	GetDescription(void);
		MEDIA_KIT_15::node_kind	GetKind(void);
	protected:
	
	private:
		friend class 			EffectAddOn;
		friend class			EffectNode;

		
		BString					fName;
		BString					fDescription;
		BString					fCopyright;
		MEDIA_KIT_15::node_kind	fKind;
		media_format			fConnectedFormat;
	
	 // spacing
	private:
		virtual void			_ReservedMethod0();
		virtual void			_ReservedMethod1();
		virtual void			_ReservedMethod2();
		virtual void			_ReservedMethod3();
		virtual void			_ReservedMethod4();
		virtual void			_ReservedMethod5();
		virtual void			_ReservedMethod6();
		virtual void			_ReservedMethod7();
		virtual void			_ReservedMethod8();
		virtual void			_ReservedMethod9();
		uint32					_ReservedData[10];
};

#endif