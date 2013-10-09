/* Copyright (C) Eric Dybsand, 2001. 
 * All rights reserved worldwide.
 *
 * This software is provided "as is" without express or implied
 * warranties. You may freely copy and compile this source into
 * applications you distribute provided that the copyright text
 * below is included in the resulting source code, for example:
 * "Portions Copyright (C) Eric Dybsand, 2001"
 */
// FSMclass.cpp: implementation of the FSMclass class.  Unoptimized
//
//////////////////////////////////////////////////////////////////////

#include "FSM.h"
#include "FSMclass.h"
#include "FSMstate.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////
// StateTransition() - perform a state transition based on input value
// passed and the current state, and return m_iCurrentState if there 
// is no matching output state for the input value, or 0 if there is
// some type of problem (like the current state not found)
//////////////////////////////////////////////////////////////////////

int FSMclass::StateTransition( int iInput )
{
	// the current state of the FSM must be set to have a transition
	if( !m_iCurrentState )
		return m_iCurrentState;

	// get the pointer to the FSMstate object that is the current state
	FSMstate *pState = GetState( m_iCurrentState );
	if( pState == NULL )
	{
		// signal that there is a problem
		m_iCurrentState = 0;
		return m_iCurrentState;
	}

	// now pass along the input transition value and let the FSMstate
	// do the really tough job of transitioning for the FSM, and save
	// off the output state returned as the new current state of the
	// FSM and return the output state to the calling process
	m_iCurrentState = pState->GetOutput( iInput );
	return m_iCurrentState;
}

//////////////////////////////////////////////////////////////////////
// GetState() - return the FSMstate object pointer referred to by the
// state ID passed
//////////////////////////////////////////////////////////////////////

FSMstate *FSMclass::GetState( int iStateID )
{
	FSMstate *pState = NULL;
	State_Map::iterator it;

	// try to find this FSMstate in the map
	if( !m_map.empty() )
	{
		it = m_map.find( iStateID );
		if( it != m_map.end() )
			pState = (FSMstate *)((*it).second);
	}
	return( pState );
}

//////////////////////////////////////////////////////////////////////
// AddState() - add a FSMstate object pointer to the map
//////////////////////////////////////////////////////////////////////

void FSMclass::AddState( FSMstate *pNewState )
{
	FSMstate *pState = NULL;
	State_Map::iterator it;

	// try to find this FSMstate in the map
	if( !m_map.empty() )
	{
		it = m_map.find( pNewState->GetID() );
		if( it != m_map.end() )
			pState = (FSMstate *)((*it).second);
	}

	// if the FSMstate object pointer is already in the map, return
	if( pState != NULL )
		return;

	// otherwise put the FSMstate object pointer into the map
	m_map.insert( SM_VT(pNewState->GetID(), pNewState) );
}

//////////////////////////////////////////////////////////////////////
// DeleteState() - delete a FSMstate object pointer from the map
//////////////////////////////////////////////////////////////////////

void FSMclass::DeleteState( int iStateID )
{
	FSMstate *pState = NULL;
	State_Map::iterator it;

	// try to find this FSMstate in the map
	if( !m_map.empty() )
	{
		// get the iterator object of the FSMstate object pointer
		it = m_map.find( iStateID );
		if( it != m_map.end() )
			pState = (FSMstate *)((*it).second);
	}

	// confirm that the FSMstate is in the map
	if( pState != NULL &&
		pState->GetID() == iStateID )
	{
		m_map.erase( it );	// remove it from the map
		delete pState;		// delete the object itself
	}
}

//////////////////////////////////////////////////////////////////////
// FSMclass() - Construction method
//////////////////////////////////////////////////////////////////////

FSMclass::FSMclass( int iStateID )
{
	m_iCurrentState = iStateID;
}

//////////////////////////////////////////////////////////////////////
// ~FSMclass() - Destruction method
//////////////////////////////////////////////////////////////////////

FSMclass::~FSMclass()
{
	FSMstate *pState = NULL;
	State_Map::iterator it;

	// only perform this if there are pointers in the map
	if( !m_map.empty() )
	{
		// first delete any FSMstate objects in the map
		for( it = m_map.begin(); it != m_map.end(); ++it )
		{
			pState = (FSMstate *)((*it).second);
			if( pState != NULL )
				delete pState;
		}
		// let the map dtor() erase the actual pointer out of the map
	}
}

//////////////////////////////////////////////////////////////////////
// CreateGlobalFSM() - Cria a FSM espec�fica do projeto 
//////////////////////////////////////////////////////////////////////
void FSMclass::CreateGlobalFSM()
{
	FSMstate *pFSMstate = NULL;
	
	// Cria o STATE_ID_LARGADA
	try
	{
		pFSMstate = new FSMstate(STATE_ID_LARGADA, 1);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_SINALVERDE, STATE_ID_RACING);
	this->AddState(pFSMstate);


	// Cria o STATE_ID_RACING
	try
	{
		pFSMstate = new FSMstate(STATE_ID_RACING, 4);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_BANDEIRADA, STATE_ID_CHEGADA);
	pFSMstate->AddTransition(INPUT_ID_SETOR_OUT, STATE_ID_OFFTRACK);
	pFSMstate->AddTransition(INPUT_ID_ULTRAPASSAR_ON, STATE_ID_RACING_ULTRAPASSAGEM);
	pFSMstate->AddTransition(INPUT_ID_OBSTACULO_ON, STATE_ID_RACING_DESVIO);
	this->AddState(pFSMstate);


	// Cria o STATE_ID_CHEGADA
	try
	{
		pFSMstate = new FSMstate(STATE_ID_CHEGADA, 0);
	}
	catch( ... )
	{
		throw;
	}
	this->AddState(pFSMstate);
}

//////////////////////////////////////////////////////////////////////
// CreateIAFSM() - Cria a FSM espec�fica da IA
//////////////////////////////////////////////////////////////////////
void FSMclass::CreateIAFSM()
{
	FSMstate *pFSMstate = NULL;
	
	// Cria o STATE_ID_RACING_LINE
	try
	{
		pFSMstate = new FSMstate(STATE_ID_RACING_LINE, 3);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_SETOR_OUT, STATE_ID_OFFTRACK);
	pFSMstate->AddTransition(INPUT_ID_ULTRAPASSAR_ON, STATE_ID_RACING_ULTRAPASSAGEM);
	pFSMstate->AddTransition(INPUT_ID_OBSTACULO_ON, STATE_ID_RACING_DESVIO);
	this->AddState(pFSMstate);


	// Cria o STATE_ID_RACING_ULTRAPASSAGEM
	try
	{
		pFSMstate = new FSMstate(STATE_ID_RACING_ULTRAPASSAGEM, 1);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_ULTRAPASSAR_OFF, STATE_ID_RACING_LINE);
	this->AddState(pFSMstate);


	// Cria o STATE_ID_RACING_DESVIO
	try
	{
		pFSMstate = new FSMstate(STATE_ID_RACING_DESVIO, 1);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_OBSTACULO_OFF, STATE_ID_RACING_LINE);
	this->AddState(pFSMstate);
	

	// Cria o STATE_ID_OFFTRACK
	try
	{
		pFSMstate = new FSMstate(STATE_ID_OFFTRACK, 1);
	}
	catch( ... )
	{
		throw;
	}
	pFSMstate->AddTransition(INPUT_ID_SETOR_IN, STATE_ID_RACING_LINE);
	this->AddState(pFSMstate);
}

// end of FSMclass.cpp
