#include "BoardState.h"
#include <iostream>

BoardState::BoardState() :
	m_turn{ true }, m_shortCastleW{ true }, m_longCastleW{ true },
	m_shortCastleB{ true }, m_longCastleB{ true }, rowLength{ 8 }
{
	initBoard();
}

void BoardState::initBoard() {

	


	// index = row * rowLength + col, flattening 2d array
	// for row number 1, white pawns
	for (int col{ 0 }; col < 8; ++col) {
		// row = 1, rowLength = 8
		int index = 1 * 8 + col;
		m_board[index] = 'P';
	}

	//// for row number 6, black pawns

	for (int col{ 0 }; col < 8; ++col) {
		// row = 6, rowLength = 8
		int index = 6 * 8 + col;
		m_board[index] = 'p';
	}

	//// for row number 0, white's home row
	const char* startRow{ "RNBKQBNR" };
	for (int col{ 0 }; col < 8; ++col) {
		// row = 0, rowLength = 8
		int index = col;
		m_board[index] = startRow[col];
	}

	//// for row number 7, black's home row
	startRow = "rnbkqbnr";
	for (int col{ 0 }; col < 8; ++col) {
		// row = 7, rowLength = 8
		int index = 7 * 8 + col;
		m_board[index] = startRow[col];
	}

	//// for rows 2 through 5, empty squares
	for (int row{ 2 }; row < 6; ++row) {
		for (int col{ 0 }; col < 8; ++col) {
			int index{ row * 8 + col };
			// empty square
			m_board[index] = '-';
		}
	}
}

void BoardState::printBoardReverse() {
	int line{ 0 };
	for (char c : m_board) {
		std::cout << c << ' ';
		++line;
		if (line % 8 == 0) {
			std::cout << '\n';
		}
	}
	if (m_turn)
		std::cout << "White to play\n";
	else
		std::cout << "Black to play\n";
}

void BoardState::printBoard() {
	int line{ 0 };
	int lastIndex{ static_cast<int>(m_board.size()) - 1 };
	for (int index{ lastIndex }; index > -1; --index) {
		std::cout << m_board[index] << ' ';
		++line;
		if (line % 8 == 0) {
			std::cout << '\n';
		}
	}
}

void BoardState::newBoardState(const BoardState& board) {
	*this = board;
}

//void BoardState::playMove(const ChessMove& move) {
//	// index = row * rowLength + col, flattening 2d array
//	int startIndex{ move.m_startIndex };
//	int endIndex{ move.m_endIndex};
//
//	if (move.type == move.shortCastling) {
//		int nextTo{ 1 };
//		if (move.onMove == true)
//			m_shortCastleW = false;
//		else {
//			m_shortCastleB = false;
//			nextTo = -1;
//		}
//		m_board[endIndex] = m_board[startIndex];
//		m_board[startIndex] = '-';
//		m_board[startIndex + nextTo] = m_board[endIndex + nextTo];
//		m_board[endIndex + nextTo] = '-';
//	}
//	else if (move.type == move.longCastling) {
//		int nextTo{ -1 };
//		if (move.onMove == true)
//			m_longCastleW = false;
//		else {
//			m_longCastleB = false;
//			nextTo = 1;
//		}
//		m_board[endIndex] = m_board[startIndex];
//		m_board[startIndex] = '-';
//
//		m_board[startIndex + nextTo] = m_board[endIndex + 2 * nextTo];
//		m_board[endIndex + 2 * nextTo] = '-';
//	}
//	else if (move.type == move.enPasant) {
//		m_board[endIndex] = m_board[startIndex];
//		m_board[startIndex] = '-';
//
//		m_board[move.enPasantIndex] = '-';
//	}
//	
//	
//	m_board[endIndex] = m_board[startIndex];
//	m_board[startIndex] = '-';
//	m_turn = !m_turn;
//}
////
