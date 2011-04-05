/****************************************************************************
** Copyright (C) 2011 Luka Horvat <redreaper132 at gmail.com>
** Copyright (C) 2011 Edward Lii <edward_iii at myway.com>
** Copyright (C) 2011 O. Bahri Gordebak <gordebak at gmail.com>
**
**
** This file may be used under the terms of the GNU General Public
** License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
****************************************************************************/
#ifndef BLOCK_H
#define BLOCK_H

#include <SDL/SDL.h>
#include "Globals.h"
#include "GameObjects.h"
#include <vector>

class Game;

class Block : public GameObject
{
private:
	SDL_Surface *surface2;
	int m_t;
	int m_t_save;

	//for moving objects
	SDL_Rect box_base;
	std::vector<SDL_Rect> MovingPos;
	int m_dx,m_dy,m_x_save,m_y_save;
	//over

public:

	Block(int x, int y, int type, Game *objParent);
	~Block();

	virtual SDL_Rect get_box_base();

	void show();

	virtual void save_state();
	virtual void load_state();
	virtual void reset();
	virtual void play_animation(int flags);
	virtual void OnEvent(int nEventType);
	virtual int QueryProperties(int nPropertyType,Player* obj);
	virtual void GetEditorData(std::vector<std::pair<std::string,std::string> >& obj);
	virtual void SetEditorData(std::map<std::string,std::string>& obj);
	virtual void move();
};

#endif