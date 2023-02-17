#pragma once
#include "ChessMove.h"
#include <vector>
#include <utility>
#include <array>
#include <cmath>
// index = row * rowLength + col

class BoardState
{
public:
	BoardState();
	//bool kingInCheck();
	void initBoard();
	void printBoard();
	void newBoardState(const BoardState& board);
	//void playMove(const ChessMove& move);
	void printBoardReverse();

	std::array<char, 64> m_board{};
	int rowLength{};

	// true if white on turn
	bool m_turn{};

	// short and long white castling rights
	bool m_shortCastleW{};
	bool m_longCastleW{};

	// short and long black castling rights
	bool m_shortCastleB{};
	bool m_longCastleB{};

	int whiteKingIndex{ 3 };
	int blackKingIndex{ 59 };

	//std::stack<ChessMove> movesPlayed{};
};

