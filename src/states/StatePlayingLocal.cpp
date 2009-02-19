/*=============================================================================
XMOTO

This file is part of XMOTO.

XMOTO is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

XMOTO is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XMOTO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
=============================================================================*/

#include "StatePlayingLocal.h"
#include "StatePause.h"
#include "../Game.h"
#include "../XMSession.h"
#include "../helpers/Log.h"
#include "../GameText.h"
#include "StateMessageBox.h"
#include "StateFinished.h"
#include "StateDeadJust.h"
#include "StateDeadMenu.h"
#include "../Sound.h"
#include "../xmscene/Camera.h"
#include "../xmscene/Bike.h"
#include "../CameraAnimation.h"
#include "../Universe.h"
#include "../Trainer.h"
#include "../SysMessage.h"
#include "../Renderer.h"
#include "../xmscene/BikeController.h"
#include "../LuaLibGame.h"

#define MINIMUM_VELOCITY_TO_GET_MAXIMUM_DEATH_SOUND 70.0

StatePlayingLocal::StatePlayingLocal(Universe* i_universe):
  StatePlaying(i_universe)
{
  m_name = "StatePlayingLocal";
  m_gameIsFinished = false;

  /* prepare stats */
  makeStatsStr();

  StateManager::instance()->registerAsObserver("OPTIONS_UPDATED", this);

  if(XMSession::instance()->debug() == true) {
    StateManager::instance()->registerAsEmitter("STATS_UPDATED");
    StateManager::instance()->registerAsEmitter("LEVELS_UPDATED");
  }
}

StatePlayingLocal::~StatePlayingLocal()
{
  StateManager::instance()->unregisterAsObserver("OPTIONS_UPDATED", this);
}

void StatePlayingLocal::enter()
{
  StatePlaying::enter();

  m_gameIsFinished = false;

  std::string v_level_name;
  try {
    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	v_level_name = m_universe->getScenes()[i]->getLevelSrc()->Name();
	m_universe->getScenes()[i]->playLevel();
      }
    }
  }
  catch(Exception &e) {
    LogWarning("level '%s' cannot be loaded", v_level_name.c_str());

    char cBuf[256];
    snprintf(cBuf, 256, GAMETEXT_LEVELCANNOTBELOADED, v_level_name.c_str());

    StateMessageBox* v_msgboxState = new StateMessageBox(this, cBuf, UI_MSGBOX_OK);
    v_msgboxState->setId("ERROR");
    StateManager::instance()->pushState(v_msgboxState);
  }

  // read keys for more reactivity
  dealWithActivedKeys();

  // reset trainer mode use
  Trainer::instance()->resetTrainerUse();
}

void StatePlayingLocal::leave()
{
  if(m_universe != NULL) {
    for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
      m_universe->getScenes()[i]->setInfos("");
    }
  }

  if(GameApp::instance()->isRequestingEnd()) {
    // when end if forced, update stats as aborted
    if(m_universe != NULL) {
      if(m_universe->getScenes().size() == 1) {
	if(m_universe->getScenes()[0]->Players().size() == 1) {
	  if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	     m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	    
	    xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  }
	}
      }
    }
  }
}

void StatePlayingLocal::enterAfterPop()
{
  StatePlaying::enterAfterPop();

  // recheck keys
  dealWithActivedKeys();
  m_fLastPhysTime = GameApp::getXMTime();
}

bool StatePlayingLocal::update()
{
  if(StatePlaying::update() == false)
    return false;

  if(isLockedScene() == false) {
    bool v_all_dead       = true;
    bool v_one_still_play = false;
    bool v_one_finished   = false;
    
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_all_dead = false;
	  }
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished()) {
	    v_one_finished = true;
	  }
	  
	  if(m_universe->getScenes()[j]->Players()[i]->isFinished() == false &&
	     m_universe->getScenes()[j]->Players()[i]->isDead() == false) {
	    v_one_still_play = true;
	  }
	}
      }
    }
    
    if(m_gameIsFinished == false) {
      if(v_one_still_play == false || XMSession::instance()->MultiStopWhenOneFinishes()) { // let people continuing when one finished or not
	if(v_one_finished) {
	  /* You're done maaaan! :D */
	  onOneFinish();
	  m_gameIsFinished = true;
	} else if(v_all_dead) {
	  /* You're dead maan! */
	  onAllDead();
	  m_gameIsFinished = true;
	}
      }
    }
  }

  return true;
}

void StatePlayingLocal::xmKey(InputEventType i_type, const XMKey& i_xmkey) {
  if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_ESCAPE, KMOD_NONE)) {
    if(isLockedScene() == false) {
      /* Escape pauses */
      m_displayStats = true;
      StateManager::instance()->pushState(new StatePause(m_universe, this));
    }
  }

#if defined(ENABLE_DEV)
  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP0, KMOD_LCTRL)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
	for(unsigned int i=0; i<m_universe->getScenes()[j]->Players().size(); i++) {
	  if(m_universe->getScenes()[j]->Cameras().size() > 0) {
	    m_universe->TeleportationCheatTo(i, Vector2f(m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionX(),
							 m_universe->getScenes()[j]->Cameras()[0]->getCameraPositionY()));
	  }
	}
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_KP0, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        Trainer::instance()->storePosition( m_universe->getScenes()[j]->getLevelSrc()->Id(),
                                            m_universe->getScenes()[j]->getPlayerPosition(0) );
          //TODO: bool getPlayerFaceDir (int i_player)
        char sysmsg[256];
        snprintf(sysmsg, 256, SYS_MSG_TRAIN_STORED, Trainer::instance()->getMaxRestoreIndex()+1);
        SysMessage::instance()->displayText(sysmsg);
      }
    }
  }

  // TRAINER
  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_BACKSPACE, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getCurrentRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_MINUS, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getPreviousRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }

  else if(i_type == INPUT_DOWN && i_xmkey == XMKey(SDLK_PLUS, KMOD_NONE)) {
    if(m_universe != NULL) {
      for(unsigned int j=0; j<m_universe->getScenes().size(); j++) {
        if( Trainer::instance()->isRestorePositionAvailable( m_universe->getScenes()[j]->getLevelSrc()->Id() ) ) {
          Vector2f pos = Trainer::instance()->getNextRestorePosition( m_universe->getScenes()[j]->getLevelSrc()->Id() );
          m_universe->TeleportationCheatTo(0, pos );
          char sysmsg[256];
          snprintf(sysmsg, 256, SYS_MSG_TRAIN_RESTORING, Trainer::instance()->getCurrentRestoreIndex()+1,
                                                         Trainer::instance()->getMaxRestoreIndex()+1);
          SysMessage::instance()->displayText(sysmsg);
        } else {
          SysMessage::instance()->displayText(SYS_MSG_TRAIN_NO_RESTORE_AVAIL);
        }
      }
    }
  }
#endif

  else {
    if(i_type == INPUT_DOWN) {
      if(m_autoZoom == false){
	// to avoid people changing direction during the autozoom
	handleInput(INPUT_DOWN, i_xmkey);
      }
    } else {
      handleInput(INPUT_UP, i_xmkey);
    }
    StatePlaying::xmKey(i_type, i_xmkey);
  }
}

void StatePlayingLocal::onOneFinish() {
  GameApp*  pGame = GameApp::instance();

  /* finalize the replay */
  if(m_universe != NULL) {
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(true);
    }
  }
  
  /* update profile and stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	int v_finish_time = 0;
	std::string TimeStamp = pGame->getTimeStamp();
	if(m_universe->getScenes()[0]->Players()[0]->isFinished()) {
	  v_finish_time  = m_universe->getScenes()[0]->Players()[0]->finishTime();
	}
        // Updating the stats if the Trainer has not been used
        if(Trainer::instance()->trainerHasBeenUsed() == false){
            xmDatabase::instance("main")->profiles_addFinishTime(XMSession::instance()->sitekey(),
								 XMSession::instance()->profile(),
								 m_universe->getScenes()[0]->getLevelSrc()->Id(),
								 TimeStamp,
								 v_finish_time);
        }
	xmDatabase::instance("main")->stats_levelCompleted(XMSession::instance()->sitekey(),
							   XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->Players()[0]->finishTime());
	StateManager::instance()->sendAsynchronousMessage("LEVELS_UPDATED");
        StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  
  StateManager::instance()->pushState(new StateFinished(m_universe, this));
}

void StatePlayingLocal::onAllDead() {  
  if(m_universe != NULL) {	
    if(m_universe->isAReplayToSave()) {
      m_universe->finalizeReplay(false);
    }
  }
  
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	xmDatabase::instance("main")->stats_died(XMSession::instance()->sitekey(),
						 XMSession::instance()->profile(),
						 m_universe->getScenes()[0]->getLevelSrc()->Id(),
						 m_universe->getScenes()[0]->getTime());
	StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
      }
    }
  }
  
  /* Play the DIE!!! sound */
  try {
    float v_deathVolume;
    float v_maxVelocity = 0.0;

    if(m_universe != NULL) {
      for(unsigned int i=0; i<m_universe->getScenes().size(); i++) {
	for(unsigned int j=0; j<m_universe->getScenes()[i]->Players().size(); j++) {
	  if(m_universe->getScenes()[i]->Players()[j]->getBikeLinearVel() > v_maxVelocity) {
	    v_maxVelocity = m_universe->getScenes()[i]->Players()[j]->getBikeLinearVel();
	  }
	}
      }
    }
    // make deathVolume dependant of the velocity of the fastest of the players
    v_deathVolume = v_maxVelocity / MINIMUM_VELOCITY_TO_GET_MAXIMUM_DEATH_SOUND;
    if(v_deathVolume > 1.0) {
      v_deathVolume = 1.0;
    }
    Sound::playSampleByName(Theme::instance()->getSound("Headcrash")->FilePath(), v_deathVolume);
  } catch(Exception &e) {
  }
  
  if(XMSession::instance()->enableDeadAnimation()) {
    StateManager::instance()->replaceState(new StateDeadJust(m_universe));
  } else {
    StateManager::instance()->pushState(new StateDeadMenu(m_universe, true, this));
  }
}

void StatePlayingLocal::abortPlaying() {
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_abortedLevel(XMSession::instance()->sitekey(),
							   XMSession::instance()->profile(),
							   m_universe->getScenes()[0]->getLevelSrc()->Id(),
							   m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  StatePlaying::abortPlaying();
}

void StatePlayingLocal::nextLevel(bool i_positifOrder) {
  playingNextLevel(i_positifOrder);
}

void StatePlayingLocal::restartLevel(bool i_reloadLevel) {
  /* Update stats */
  if(m_universe != NULL) {
    if(m_universe->getScenes().size() == 1) {
      if(m_universe->getScenes()[0]->Players().size() == 1) {
	if(m_universe->getScenes()[0]->Players()[0]->isDead()     == false &&
	   m_universe->getScenes()[0]->Players()[0]->isFinished() == false) {
	  xmDatabase::instance("main")->stats_levelRestarted(XMSession::instance()->sitekey(),
							     XMSession::instance()->profile(),
							     m_universe->getScenes()[0]->getLevelSrc()->Id(),
							     m_universe->getScenes()[0]->getTime());
	  StateManager::instance()->sendAsynchronousMessage("STATS_UPDATED");
	}
      }
    }
  }

  restartLevelToPlay(i_reloadLevel);
}

void StatePlayingLocal::handleInput(InputEventType Type, const XMKey& i_xmkey) {
  handleControllers(Type, i_xmkey);
  handleScriptKeys(Type, i_xmkey);
}