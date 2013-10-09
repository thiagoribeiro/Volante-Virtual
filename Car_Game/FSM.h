#ifndef _FSM_H
#define _FSM_H

//
// these headers declare STL support for the application
//

#include <list>
#include <vector>
#include <deque>
#include <map>
#include <queue>
#include <utility>
#include <iterator>
#include <algorithm>
#include <functional>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <strstream>
#include <sstream>
#include <exception>
#include <cmath>

using namespace std;


//-------------------------
// constantes dos estados
//-------------------------
enum
{
	STATE_ID_LARGADA = 1,
	STATE_ID_RACING,
	STATE_ID_RACING_LINE,
	STATE_ID_RACING_ULTRAPASSAGEM,
	STATE_ID_RACING_DESVIO,
	STATE_ID_OFFTRACK,
	STATE_ID_CHEGADA,
	INPUT_ID_SINALVERDE,
	INPUT_ID_BANDEIRADA,
	INPUT_ID_SETOR_IN,
	INPUT_ID_SETOR_OUT,
	INPUT_ID_ULTRAPASSAR_ON,
	INPUT_ID_ULTRAPASSAR_OFF,
	INPUT_ID_OBSTACULO_ON,
	INPUT_ID_OBSTACULO_OFF,
	NUMBER_OF_IDS_USED
};

#endif
