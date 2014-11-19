#include "RolloutAgent.h"

RolloutAgent::RolloutAgent()
{
	Agent();
}

Action* RolloutAgent::getAction(Tetris *board)
{
	vector<Action*> bestActions;
	float bestValue = -99999999;

	//For all valid rotations
	for (int r = 0; r < NUM_ROTATIONS; r++) {
		Rotation rot = (Rotation) r;

		//Make sure that we only check valid columns
		int maxColumn = board->highestValidColWithRot(rot) + 1;

		//For all valid columns for each rotation
		for (int col = 0; col < maxColumn; col++) {
			//Create the action for this move (will be cleaned up by playing it)
			
			float actVal = 0;
			
			for (int w = 0; w < W; w++) {
				Action *a = new Action(rot, col);

				//Copy the board -- this makes it have random next pieces
				Tetris *trajectorySim = board->gameCopy();

				// cout << "NEW TRAJ" << endl;

				//This also plays the action
				float trajValue = valueOfActionOnBoard(a, trajectorySim);

				for (int k = 0; k < K; k++) {
					// trajectorySim->printBoard();
					Action *heuristicAct = getActionHeuristic(trajectorySim);
					// trajectorySim->playAction(heuristicAct, false);
					trajValue += pow(GAMMA, k+1) * valueOfActionOnBoard(heuristicAct, trajectorySim);
				}

				actVal += trajValue;
				// actVal += valueBetweenBoards(board, trajectorySim);
				// trajectorySim->printBoard();
			}
			
			//Average over the W K-length trajectories
			actVal /= W;
			// actionSim->printBoard();
			// cout << "PLAY WITH ROW: " << r << " AND COL: " << col << endl;
			// cout << "SCORE: " << actVal << endl << endl;
			// exit(1);
			if (actVal > bestValue) {
				bestValue = actVal;
				foundNewBestAction(bestActions, rot, col);
			} else if (actVal == bestValue) {
				foundTiedAction(bestActions, rot, col);
			}
		}
	}

	//Choose the best action
	Action *a = pickRandomAction(bestActions);

	//Clean up the actions
	clearActionList(bestActions);

	//Play the action
	return a;
}

Action* RolloutAgent::getActionHeuristic(Tetris *board)
{		
	float bestValue = -99999999;

	vector<Action*> bestActions;

	//For all valid rotations
	for (int r = 0; r < NUM_ROTATIONS; r++) {
		Rotation rot = (Rotation) r;

		//Make sure that we only check valid columns
		int maxColumn = board->highestValidColWithRot(rot) + 1;

		//For all valid columns for each rotation
		for (int col = 0; col < maxColumn; col++) {
			//Create the action for this move (will be cleaned up by playing it)
			Action *a = new Action(rot, col);

			//Copy the board
			Tetris *sim = board->gameCopy();

			float value = valueOfActionOnBoard(a, sim);

			if (value > bestValue) {
				bestValue = value;
				foundNewBestAction(bestActions, rot, col);
			} else if (value == bestValue) {
				foundTiedAction(bestActions, rot, col);
			}

			delete sim;
		}
	}

	//Choose the best action
	Action *a = pickRandomAction(bestActions);

	//Clean up the actions
	clearActionList(bestActions);

	//Play the action
	return a;
}

float RolloutAgent::valueOfActionOnBoard(Action *a, Tetris *sim)
{
			//Save current game stats
			int prevLines      = sim->getLinesCleared();
			int prevHeight     = sim->maxBoardHeight();
			int prevHoles      = sim->holesInBoard();
			int prevBlocked    = sim->topDownBlocked();
			int prevAggBlocked = sim->aggregateTopDownBlocked();

			//Play the action
			sim->playAction(a, false);

			//Observe new stats
			int linesCleared    = sim->getLinesCleared() - prevLines;
			int heightGain      = prevHeight - sim->maxBoardHeight();
			int newHoles        = sim->holesInBoard() - prevHoles;
			int topBlocked      = sim->topDownBlocked() - prevBlocked;
			int aggTopBlocked   = sim->aggregateTopDownBlocked() - prevAggBlocked;
			bool lost		    = sim->isLost();

			return valueOfAction(linesCleared, heightGain, newHoles, topBlocked, aggTopBlocked, lost);
}

float RolloutAgent::valueBetweenBoards(Tetris *board1, Tetris *board2)
{
	int linesCleared    = board2->getLinesCleared() - board1->getLinesCleared();
	int heightGain      = board1->maxBoardHeight() - board2->maxBoardHeight();
	int newHoles        = board2->holesInBoard() - board1->holesInBoard();
	int topBlocked      = board2->topDownBlocked() - board1->topDownBlocked();
	int aggTopBlocked   = board2->aggregateTopDownBlocked() - board1->aggregateTopDownBlocked();
	bool lost		    = board2->isLost();

	return valueOfAction(linesCleared, heightGain, newHoles, topBlocked, aggTopBlocked, lost);
}

float RolloutAgent::valueOfAction(int linesCleared, int heightGain, int newHoles, int topDownBlocked, int aggTopBlocked, bool lost)
{
	if (lost) {
		return -10000;
	} else {
		return (50 * linesCleared) + (0 * newHoles) + (-10 * topDownBlocked) + (0 * aggTopBlocked) + (-25 * heightGain);
	}
}

Action* RolloutAgent::pickRandomAction(vector<Action*> &actions)
{
	assert(actions.size() > 0);
	
	//Randomly pick an index and get the action
	int actIndex = rand() % actions.size();
	Action *a = actions.at(actIndex);

	//Make a copy since all of the values in the vector will be cleaned up
	return new Action(a->rotation, a->column);
}

void RolloutAgent::clearActionList(vector<Action*> &actions)
{
	for(vector<Action*>::iterator it = actions.begin(); it != actions.end(); it++) {
		Action *a = (*it);
		delete a;
	}
	actions.clear();
}

void RolloutAgent::foundNewBestAction(vector<Action*> &actions, Rotation rot, int col)
{
	clearActionList(actions);
	Action *a = new Action(rot, col);
	actions.push_back(a);
}

void RolloutAgent::foundTiedAction(vector<Action*> &actions, Rotation rot, int col)
{
	Action *a = new Action(rot, col);
	actions.push_back(a);
}