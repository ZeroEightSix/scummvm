/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "got/game/init.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "got/events.h"
#include "got/game/back.h"
#include "got/gfx/image.h"
#include "got/sound.h"
#include "got/utils/file.h"
#include "got/vars.h"

namespace Got {

void setupPlayer() {
	_G(thor_info).clear();
	_G(thor_info)._inventory = 0;
	if (_G(area) > 1)
		_G(thor_info)._inventory |= APPLE_MAGIC + LIGHTNING_MAGIC;
	if (_G(area) > 2)
		_G(thor_info)._inventory |= BOOTS_MAGIC + WIND_MAGIC;

	_G(thor)->_health = 150;
	_G(thor_info)._magic = _G(area) > 1 ? 150 : 0;
	_G(thor_info)._jewels = 0;
	_G(thor_info)._score = 0;
	_G(thor_info)._keys = 0;
	_G(thor_info)._lastItem = 0;
	_G(thor_info)._object = 0;
	_G(thor_info)._objectName = nullptr;
	_G(thor)->_lastX[0] = _G(thor)->_x;
	_G(thor)->_lastX[1] = _G(thor)->_x;
	_G(thor)->_lastY[0] = _G(thor)->_y;
	_G(thor)->_lastY[1] = _G(thor)->_y;
	_G(thor_info)._lastIcon = (6 * 20) + 8;
	_G(thor_info)._lastScreen = 23;
	_G(thor)->_dir = 1;

	switch (_G(area)) {
	case 1:
		_G(thor)->_x = 152;
		_G(thor)->_y = 96;
		break;
	case 2:
		_G(thor)->_x = 32;
		_G(thor)->_y = 32;
		break;
	case 3:
		_G(thor)->_x = 272;
		_G(thor)->_y = 80;
		break;
	default:
		break;
	}
}

void initGame() {
	loadStandardActors();
	setupPlayer();

	if (_G(demo)) {
		g_vars->setArea(1);
		_G(thor)->_health = 100;
		_G(thor_info)._magic = 100;
		_G(thor_info)._jewels = 463;
		_G(thor_info)._score = 12455;
		_G(setup)._difficultyLevel = 0;
		_G(thor_info)._inventory = 1 + 2;
		_G(current_level) = 54;
		_G(thor_info)._selectedItem = 2;

		File f("DEMO");
		_G(demoKeys).clear();
		for (int i = 0; i < DEMO_LEN; ++i)
			_G(demoKeys).push(f.readByte());

		// Drop the first six bytes off. This comprises of
		// the "part I" display in the original, and ScummVM
		// doesn't do demo key processing until the game view
		for (int i = 0; i < 6; ++i)
			_G(demoKeys).pop();
	}

	_G(thor)->_moveCountdown = 6;

	// Load level data
	_G(new_level) = _G(current_level);
	_G(scrn).load(_G(current_level));
	showLevel(_G(current_level));

	if (!_G(auto_load)) {
		_G(sound).musicPlay(_G(levelMusic), true);
	}

	g_vars->resetEndgameFlags();
	_G(startup) = false;
}

int setupBoss(const int num) {
	if (_G(boss_loaded) == num)
		return 1;

	if (_G(boss_loaded)) {
		for (int rep = 0; rep < 3; rep++) {
			if (_G(boss_sound[rep]))
				free(_G(boss_sound[rep]));
			if (_G(boss_pcsound[rep]))
				free(_G(boss_pcsound[rep]));
		}
	}

	Common::String ressourceName = Common::String::format("BOSSV%d1", num);
	_G(boss_sound[0]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_sound[0]))
		return 0;
	_G(dig_sound[NUM_SOUNDS - 3]) = _G(boss_sound[0]);

	ressourceName = Common::String::format("BOSSV%d2", num);
	_G(boss_sound[1]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_sound[1]))
		return 0;
	_G(dig_sound[NUM_SOUNDS - 2]) = _G(boss_sound[1]);

	ressourceName = Common::String::format("BOSSV%d3", num);
	_G(boss_sound[2]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_sound[2]))
		return 0;
	_G(dig_sound[NUM_SOUNDS - 1]) = _G(boss_sound[2]);

	Common::String prefix = (num == 2) ? "BOSSP1" : Common::String::format("BOSSP%d", num);
	ressourceName = prefix + "1";
	_G(boss_pcsound[0]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_pcsound[0]))
		return 0;

	_G(pc_sound[NUM_SOUNDS - 3]) = _G(boss_pcsound[0]);
	_G(pc_sound[NUM_SOUNDS - 3][0]) = 0;
	_G(pc_sound[NUM_SOUNDS - 3][1]) = 0;

	Common::File f;
	if (!f.open(Common::Path(ressourceName)))
		return 0;
	_G(pcsound_length[NUM_SOUNDS - 3]) = f.size();
	f.close();

	ressourceName = prefix + "2";
	_G(boss_pcsound[1]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_pcsound[1]))
		return 0;

	_G(pc_sound[NUM_SOUNDS - 2]) = _G(boss_pcsound[1]);
	_G(pc_sound[NUM_SOUNDS - 2][0]) = 0;
	_G(pc_sound[NUM_SOUNDS - 2][1]) = 0;

	if (!f.open(Common::Path(ressourceName)))
		return 0;
	_G(pcsound_length[NUM_SOUNDS - 2]) = f.size();
	f.close();

	ressourceName = prefix + "3";
	_G(boss_pcsound[2]) = (byte *)resourceAllocRead(ressourceName);
	if (!_G(boss_pcsound[2]))
		return 0;
	_G(pc_sound[NUM_SOUNDS - 1]) = _G(boss_pcsound[2]);
	_G(pc_sound[NUM_SOUNDS - 1][0]) = 0;
	_G(pc_sound[NUM_SOUNDS - 1][1]) = 0;

	if (!f.open(Common::Path(ressourceName)))
		return 0;
	_G(pcsound_length[NUM_SOUNDS - 1]) = f.size();
	f.close();

	_G(boss_loaded) = num;
	return 1;
}

} // namespace Got
