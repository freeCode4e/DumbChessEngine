#include "GameState.h"
#include "Square.h"
#include "MoveScore.h"
#include "../../HEADERS/Random/myRandom.h"
#include <chrono>

#include <utility>
#include <algorithm>
#include <string>
#include <string_view>
#include <functional>
#include <map>
#include <iostream>

using namespace std::chrono;



GameState::GameState() :
	BoardState(), ChessMove() {
	stackOfPlayedMoves.push_back({ *this, *this });
	for (int i{ 0 }; i < pieceValues.size(); ++i) {
		map[whitePieces[i]] = pieceValues[i];
		map[blackPieces[i]] = pieceValues[i];
	}
}

void GameState::tryMove(int startIndex, int endIndex ) {
	m_board[endIndex] = m_board[startIndex];
	m_board[startIndex] = '-';
}

void GameState::playMove() {
	// index = row * rowLength + col, flattening 2d array
	int startIndex{ m_startIndex };
	int endIndex{ m_endIndex };

	// convert move to chessMove so that we store it.

	m_board[endIndex] = m_board[startIndex];
	m_board[startIndex] = '-';

	if (!m_turn) {
		if (m_shortCastleW && isBlackPiece(0))
			m_shortCastleW = false;
		if (m_longCastleW && isBlackPiece(7))
			m_longCastleW = false;
 	}
	else {
		if (m_shortCastleB && isWhitePiece(56))
			m_shortCastleB = false;
		if (m_longCastleB && isWhitePiece(63))
			m_longCastleB = false;
	}

	if (type == shortCastling) {
		int nextTo{ -1 };
		if (onMove == true)
		{
			whiteKingIndex = endIndex;
			m_shortCastleW = false;
			m_longCastleW = false;
		}
		else
		{
			blackKingIndex = endIndex;
			m_shortCastleB = false;
			m_longCastleB = false;
		}
		
		m_board[startIndex + nextTo] = m_board[endIndex + nextTo];
		m_board[endIndex + nextTo] = '-';
	}
	else if (type == longCastling) {
		int nextTo{ 1 };
		if (onMove == true)
		{
			whiteKingIndex = endIndex;
			m_longCastleW = false;
			m_shortCastleW = false;
		}
		else
		{
			blackKingIndex = endIndex;
			m_longCastleB = false;
			m_shortCastleB = false;
		}
		m_board[startIndex + nextTo] = m_board[endIndex + 2 * nextTo];
		m_board[endIndex + 2 * nextTo] = '-';
	}
	else if (type == king) {
		if (onMove == true) {
			whiteKingIndex = endIndex;
			m_shortCastleW = false;
			m_longCastleW = false;
		}
		else {
			blackKingIndex = endIndex;
			m_shortCastleB = false;
			m_longCastleB = false;
		}
	}
	else if (type == rook) {
		if (onMove) {
			if(startIndex == 0)
				m_shortCastleW = false;
			else if(startIndex == 7)
				m_longCastleW = false;
		}
		else {
			if(startIndex == 56)
				m_shortCastleB = false;
			else if(startIndex == 63)
				m_longCastleB = false;
		}
	}
	else if (type == enPasant) {
		m_board[enPasantIndex] = '-';
	}
	else if (type == promotion) {
		m_board[endIndex] = promotionPiece;
	}
	m_turn = !m_turn;
	stackOfPlayedMoves.push_back({ *this, *this });
}
//----------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------

void GameState::undoMove() {
	stackOfPlayedMoves.pop_back();
	newChessMove(stackOfPlayedMoves.back().first);
	newBoardState(stackOfPlayedMoves.back().second);
}
void GameState::playGame() {
	bool humanPlaysFirst{ true };
	std::cout << "1. human vs computer\n";
	std::cout << "2. computer vs human\n";
	int choice{};
	std::cin >> choice;
	if (choice == 2)
		humanPlaysFirst = false;

	MoveScore moveScor{};
	while (!gameOver()) {
		getAvMoves();
		printBoardReverse();
		//printAvMoves();
		if (humanPlaysFirst) {
			getMove();
			humanPlaysFirst = false;
		}
		else {
			auto start = high_resolution_clock::now();
			moveScor = computerMove();
			auto stop = high_resolution_clock::now();
			auto duration = duration_cast<microseconds>(stop - start);
			std::cout << duration.count() / 1000'000.0;
			std::pair<int, int> move{ moveScor.move };
			newChessMove(move.first, move.second, *this);
			humanPlaysFirst = true;
		}

		playMove();
		if (gameOver())
			break;
		//undoMove();
		//newChessMove(startIndex, endIndex, *this);
		std::cout << '\n' << "Eval: " << moveScor.score<< '\n';
		std::cout << "Positions searched: " << perftPositions << '\n';
	}
	printBoardReverse();
	if (isCheckmate()) {
		if (m_turn)
			std::cout << "black wins";
		else
			std::cout << "white wins";
	}
	else if(isStalemate()) {
		std::cout << "It's a draw by stalemate";
	}
	else if (isThreFold()) {
		std::cout << "It's a draw by three-fold repetition";
	}
	else {
		std::cout << "It's a draw by insufficient material";
	}
	
}

bool GameState::gameOver() {
	return isCheckmate() || isStalemate() || isThreFold() || insuficientMaterial();
}

bool GameState::isCheckmate() {
	getAvMoves();
	if (m_turn) {
		return av_moves.empty() && SquareIsAttackedByBlack(whiteKingIndex);
	}
	else {
		return av_moves.empty() && SquareIsAttackedByWhite(blackKingIndex);
	}
}

bool GameState::isStalemate() {
	getAvMoves();
	if (m_turn) {
		return av_moves.empty() && !SquareIsAttackedByBlack(whiteKingIndex);
	}
	else {
		return av_moves.empty() && !SquareIsAttackedByWhite(blackKingIndex);
	}
}

bool GameState::isThreFold() {
	int repeated{ 0 };
	for (auto& board : stackOfPlayedMoves) {
		if (board.second.m_board == m_board)
			++repeated;
		if (repeated == 3)
			return true;
	}
	return false;
}

bool GameState::insuficientMaterial() {
	int lightPieces{ 0 };
	for (char piece : m_board) {
		if (piece == 'p' || piece == 'P' || piece == 'Q' || piece == 'q'
			|| piece == 'R' || piece == 'r')
			return false;
		if (piece == 'b' || piece == 'B' || piece == 'N' || piece == 'n')
			++lightPieces;
		if (lightPieces == 2)
			return false;
	}
	return true;
}

void GameState::getMove() {
	int startRow, startCol, endRow, endCol;
	getSquares(startRow, startCol, endRow, endCol);
	Square startSquare { startRow, startCol };
	Square endSquare  { endRow, endCol };
	int startIndex{ startRow * rowLength + startCol };
	int endIndex{ endRow * rowLength + endCol };
	std::cout << startIndex << ' ' << endIndex << '\n';
	if (!isAvMove({ startIndex, endIndex })) {
		std::cout << "Invalid move, choose again\n";
		getMove();
	}
	else {
		newChessMove(startSquare, endSquare, *this, false);
	}
}

int GameState::convertLetterToCol(char letter) {
	int count{ 0 };
	for (int i{ static_cast<int>(letters.length()) - 1 }; i > -1; --i) {
		if (letters[i] == letter) {
			return count;
		}
		++count;
	}
	return -1;
}
int GameState::convertNumberToRow(char number) {
	for (int i{ 0 }; i < numbers.size(); ++i) {
		if (numbers[i] == number) {
			return i;
		}
	}
	return -1;
}

void GameState::convert(std::string_view move, int& startRow, int& startCol, int& endRow, int& endCol) {
	startRow = convertNumberToRow(move[1]);
	endRow = convertNumberToRow(move[3]);
	startCol = convertLetterToCol(move[0]);
	endCol = convertLetterToCol(move[2]);
}

bool GameState::isValidMove(std::string_view move) {
	if (move.size() != 4) {
		return false;
	}
	
	if (!find(letters, move[0]) || !find(letters, move[2]))  {
		return false;
	}
	if (!find(numbers, move[1]) || !find(numbers, move[3])) {
		return false;
	}
	return true;
}

void GameState::getSquares(int& startRow, int& startCol, int& endRow, int& endCol) {
	std::string myMove{};
	while (true) {
		std::cout << "Enter your move: ";
		std::cin >> myMove;
		if (isValidMove(myMove))
			break;
		std::cout << "Invalid move, choose again\n";
	}
	convert(myMove, startRow, startCol, endRow, endCol);

	/*std::cout << "Entering start Square:\n";

	std::cout << "Enter Square's row: ";
	std::cin >> startRow;

	std::cout << "Enter Square's column: ";
	std::cin >> startCol;

	std::cout << "Entering end Square:\n";

	std::cout << "Enter Square's row: ";
	std::cin >> endRow;

	std::cout << "Enter Square's column: ";
	std::cin >> endCol;*/

}



void GameState::getAvMoves() {
	av_moves.clear();
	// index = row * rowLength + col, flattening 2d array
	for (int index{ 0 }; index < m_board.size(); ++index) {
		// it's a pawn can move 1 forward or 2 if on start row
		// index + rowLength gets us to the square in front of the pawn
		if (m_turn && !isWhitePiece(m_board[index]) || !m_turn && !isBlackPiece(m_board[index]))
			continue;
		addKnightMoves(index);
		addBishopMoves(index);
		addPawnMoves(index);
		addRookMoves(index);
		addQueenMoves(index);
		addKingMoves(index);
	}
}

bool GameState::isAvMove(const std::pair<int,int>& move) {
	for (auto& mov : av_moves) {
		if (move == mov) {
			return true;
		}
	}
	return false;
}

std::ostream& operator<<(std::ostream& out, std::pair<int, int>& move) {
	out << '(' << move.first << ',' << move.second << ')';
	return out;
}

void GameState::printAvMoves() {
	Square start{};
	Square end{};
	for (auto& move : av_moves) {
		if (m_board[move.first] != 'P' && m_board[move.first] != 'p')
			continue;
		getSquare(move.first, start, *this);
		getSquare(move.second, end, *this);
		std::cout << move << "\t(" << 
			letters[letters.size()-1-start.col] << start.row+1
			<< letters[letters.size()-1-end.col] << end.row+1
			<< ')' << '\n';
	}
}

void GameState::addMove(int startIndex, int endIndex) {
	//bool add{ false };
	if (m_turn) {
		newChessMove(startIndex, endIndex, *this);
		playMove();
		if (!SquareIsAttackedByBlack(whiteKingIndex))
			av_moves.push_back({ startIndex, endIndex });
		undoMove();
	}
	else {
		newChessMove(startIndex, endIndex, *this);
		playMove();
		if (!SquareIsAttackedByWhite(blackKingIndex))
			av_moves.push_back({ startIndex, endIndex });
		undoMove();
	}
}

void GameState::addPownCaptures(int index) {
	if (m_turn) {
		if (!isOnLeftmostCol(index) && isBlackPiece(m_board[index + rowLength - 1])) {
			addMove(index, index + rowLength - 1);
		}
		if (!isOnRightmostCol(index) && isBlackPiece(m_board[index + rowLength + 1])) {
			addMove(index, index + rowLength + 1);
			//av_moves.push_back({ index, index + rowLength + 1 });
		}
	}
	else {
		if (!isOnLeftmostCol(index) && isWhitePiece(m_board[index - rowLength - 1])) {
			addMove(index, index - rowLength - 1);
			//av_moves.push_back({ index, index - rowLength - 1 });
		}
		if (!isOnRightmostCol(index) && isWhitePiece(m_board[index - rowLength + 1])) {
			addMove(index, index - rowLength + 1);
			//av_moves.push_back({ index, index - rowLength + 1 });
		}
	}
	
	// maybe add enPasant too.
} 

void GameState::addEnpasant(int index) {
	if (m_turn) {
		// if on the row you need to be to play en passant
		// and there's a pawn next that just moved there
		if (index >= 4 * rowLength && index < 5 * rowLength) {
			if (!isOnRightmostCol(index) && index + 1 == m_endIndex && m_endIndex == m_startIndex - 2 * rowLength
				&& charAtStart == 'p') {
				addMove(index, index + rowLength + 1);
				//av_moves.push_back({ index, index + rowLength + 1 });
			}
			else if (!isOnLeftmostCol(index) && index - 1 == m_endIndex && m_endIndex == m_startIndex - 2 * rowLength
				&& charAtStart == 'p') {
				addMove(index, index + rowLength - 1);
				//av_moves.push_back({ index, index + rowLength - 1 });
			}
		}
	}
	else {
		if (index >= 3 * rowLength && index < 4 * rowLength) {
			if (!isOnRightmostCol(index) && index + 1 == m_endIndex && m_endIndex == m_startIndex + 2 * rowLength 
				&& charAtStart == 'P') {
				addMove(index, index - rowLength + 1);
				//av_moves.push_back({ index, index - rowLength + 1 });
			}
			else if (!isOnLeftmostCol(index) && index - 1 == m_endIndex && m_endIndex == m_startIndex + 2 * rowLength 
				&& charAtStart == 'P') {
				addMove(index, index - rowLength - 1);
				//av_moves.push_back({ index, index - rowLength - 1 });
			}
		}

	}
}

void GameState::addPawnMoves(int index) {
	// it's a pawn can move 1 forward or 2 if on start row
	// index + rowLength gets us to the square in front of the pawn
	if (m_turn) {
		if (m_board[index] == 'P') {
			if (m_board[index + rowLength] == '-')
				// if it's a pawn and the square in front is empty add that move
				addMove(index, index + rowLength);
				//av_moves.push_back({ index,index + rowLength });
			if (index < 2 * rowLength && m_board[index + 2 * rowLength] == '-'
				&& m_board[index + rowLength] == '-')
				// if it's a pawn and on second rank and the square 2 forwards is empty
				// as well as the square directly in front.
				addMove(index, index + 2 * rowLength);
				//av_moves.push_back({ index, index + 2 * rowLength });
			addPownCaptures(index);
			addEnpasant(index);
		}
	}

	else {
		if (m_board[index] == 'p') {
			if (m_board[index - rowLength] == '-')
				addMove(index, index - rowLength);
				//av_moves.push_back({ index, index - rowLength });
			if (index >= 6 * rowLength && m_board[index - 2 * rowLength] == '-'
				&& m_board[index - rowLength] == '-')
				addMove(index, index - 2 * rowLength);
				//av_moves.push_back({ index, index - 2 * rowLength });
			addPownCaptures(index);
			addEnpasant(index);
		}
	}
}

void GameState::addKnightMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'N') {
			addKnightVerticalMove(index);
			addKnightHorizontalMove(index);
		}
	}
	else {
		if (m_board[index] == 'n') {
			addKnightVerticalMove(index);
			addKnightHorizontalMove(index);
		}
	}
}

void GameState::addKnightVerticalMove(int index) {
	if (m_turn) {
		if(isValid(index + 2 * rowLength + 1)
			&& !isOnRightmostCol(index)
			&& !isWhitePiece(m_board[index + 2 * rowLength + 1]))
		{
			addMove(index, index + 2 * rowLength + 1);
			//av_moves.push_back({ index, index + 2 * rowLength + 1 });
		}
		if (isValid(index + 2 * rowLength)
			&& !isOnLeftmostCol(index)
			&& !isWhitePiece(m_board[index + 2 * rowLength - 1]))
		{
			addMove(index, index + 2 * rowLength - 1);
			//av_moves.push_back({ index, index + 2 * rowLength - 1 });
		}
		if(isValid(index - 2 * rowLength)
			&& !isOnRightmostCol(index)
			&& !isWhitePiece(m_board[index - 2 * rowLength + 1]))
		{
			addMove(index, index - 2 * rowLength + 1);
			//av_moves.push_back({ index, index - 2 * rowLength + 1 });
		}
		if(isValid(index - 2 * rowLength - 1) 
			&& !isOnLeftmostCol(index)
			&& !isWhitePiece(m_board[index - 2 * rowLength - 1]))
		{
			addMove(index, index - 2 * rowLength - 1);
			//av_moves.push_back({ index, index - 2 * rowLength - 1 });
		}
	}
	else {
		if (isValid(index + 2 * rowLength + 1)
			&& !isOnRightmostCol(index)
			&& !isBlackPiece(m_board[index + 2 * rowLength + 1]))
		{
			addMove(index, index + 2 * rowLength + 1);
			//av_moves.push_back({ index, index + 2 * rowLength + 1 });
		}
		if (isValid(index + 2 * rowLength)
			&& !isOnLeftmostCol(index)
			&& !isBlackPiece(m_board[index + 2 * rowLength - 1]))
		{
			addMove(index, index + 2 * rowLength - 1);
			//av_moves.push_back({ index, index + 2 * rowLength - 1 });
		}
		if (isValid(index - 2 * rowLength)
			&& !isOnRightmostCol(index)
			&& !isBlackPiece(m_board[index - 2 * rowLength + 1]))
		{
			addMove(index, index - 2 * rowLength + 1);
			//av_moves.push_back({ index, index - 2 * rowLength + 1 });
		}
		if (isValid(index - 2 * rowLength - 1)
			&& !isOnLeftmostCol(index)
			&& !isBlackPiece(m_board[index - 2 * rowLength - 1]))
		{
			addMove(index, index - 2 * rowLength - 1);
			//av_moves.push_back({ index, index - 2 * rowLength - 1 });
		}
	}
}
void GameState::addKnightHorizontalMove(int index) {
	if (m_turn) {
		// we should be at least 2 squares left of rightmost edge of board
		if (!isOnRightmostCol(index) && !isOnRightmostCol(index+1)
			&& isValid(index + rowLength)
			&& !isWhitePiece(m_board[index + rowLength + 2]))
		{
			addMove(index, index + rowLength + 2);
			//av_moves.push_back({ index, index + rowLength + 2 });
		}
		if (!isOnLeftmostCol(index) && !isOnLeftmostCol(index - 1)
			&& isValid(index + rowLength)
			&& !isWhitePiece(m_board[index + rowLength - 2]))
		{
			addMove(index, index + rowLength - 2);
			//av_moves.push_back({ index, index + rowLength - 2 });
		}

		if(!isOnRightmostCol(index) && !isOnRightmostCol(index+1)
			&& isValid(index - rowLength)
			&& !isWhitePiece(m_board[index - rowLength + 2]))
		{
			addMove(index, index - rowLength + 2);
			//av_moves.push_back({ index, index - rowLength + 2 });
		}
		if(!isOnLeftmostCol(index) && !isOnLeftmostCol(index-1)
			&& isValid(index - rowLength)
			&& !isWhitePiece(m_board[index - rowLength - 2]))
		{
			addMove(index, index - rowLength - 2);
			//av_moves.push_back({ index, index - rowLength - 2 });
		}
	}
	else {
		if (!isOnRightmostCol(index) && !isOnRightmostCol(index + 1)
			&& isValid(index + rowLength)
			&& !isBlackPiece(m_board[index + rowLength + 2]))
		{
			addMove(index, index + rowLength + 2);
			//av_moves.push_back({ index, index + rowLength + 2 });
		}
		if (!isOnLeftmostCol(index) && !isOnLeftmostCol(index - 1)
			&& isValid(index + rowLength)
			&& !isBlackPiece(m_board[index + rowLength - 2]))
		{
			addMove(index, index + rowLength - 2);
			//av_moves.push_back({ index, index + rowLength - 2 });
		}

		if (!isOnRightmostCol(index) && !isOnRightmostCol(index + 1)
			&& isValid(index - rowLength)
			&& !isBlackPiece(m_board[index - rowLength + 2]))
		{
			addMove(index, index - rowLength + 2);
			//av_moves.push_back({ index, index - rowLength + 2 });
		}
		if (!isOnLeftmostCol(index) && !isOnLeftmostCol(index - 1)
			&& isValid(index - rowLength)
			&& !isBlackPiece(m_board[index - rowLength - 2]))
		{
			addMove(index, index - rowLength - 2);
			//av_moves.push_back({ index, index - rowLength - 2 });
		}
	}
}


void GameState::addBishopMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'B') {
			upLeftToDownRight(index);
			downRightToUpLeft(index);
			upRightToDownLeft(index);
			downLeftToUpRight(index);
		}
	}
	else {
		if (m_board[index] == 'b') {
			upLeftToDownRight(index);
			downRightToUpLeft(index);
			upRightToDownLeft(index);
			downLeftToUpRight(index);
		}
	}
}

void GameState::addRookMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'R') {
			addRookVerticalDownMoves(index);
			addRookVerticalUpMoves(index);
			addRookLeftToRightMoves(index);
			addRookRightToLeftMoves(index);
		}
	}
	else {
		if (m_board[index] == 'r') {
			addRookVerticalDownMoves(index);
			addRookVerticalUpMoves(index);
			addRookLeftToRightMoves(index);
			addRookRightToLeftMoves(index);
		}
	}
}

void GameState::addRookVerticalDownMoves(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (isValid(index + rowLength) && !isWhitePiece(m_board[index + rowLength])) {
			addMove(startIndex, index + rowLength);
			//av_moves.push_back({ startIndex, index + rowLength });
			if (isBlackPiece(m_board[index + rowLength]))
				break;
			index = index + rowLength;
		}
	}
	else {
		while (isValid(index + rowLength) && !isBlackPiece(m_board[index + rowLength])) {
			addMove(startIndex, index + rowLength);
			//av_moves.push_back({ startIndex, index + rowLength });
			if (isWhitePiece(m_board[index + rowLength]))
				break;
			index = index + rowLength;
		}
	}
	
}
void GameState::addRookVerticalUpMoves(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (isValid(index - rowLength) && !isWhitePiece(m_board[index - rowLength])) {
			addMove(startIndex, index - rowLength);
			//av_moves.push_back({ startIndex, index - rowLength });
			if (isBlackPiece(m_board[index - rowLength]))
				break;
			index = index - rowLength;
		}
	}
	else {
		while (isValid(index - rowLength) && !isBlackPiece(m_board[index - rowLength])) {
			addMove(startIndex, index - rowLength);
			//av_moves.push_back({ startIndex, index - rowLength });
			if (isWhitePiece(m_board[index - rowLength]))
				break;
			index = index - rowLength;
		}
	}

}
void GameState::addRookLeftToRightMoves(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnRightmostCol(index) && !isWhitePiece(m_board[index + 1])) {
			addMove(startIndex, index + 1);
			//av_moves.push_back({ startIndex, index + 1 });
			if (isBlackPiece(m_board[index + 1]))
				break;
			index = index + 1;
		}
	}
	else {
		while (!isOnRightmostCol(index) && !isBlackPiece(m_board[index + 1])) {
			addMove(startIndex, index + 1);
			//av_moves.push_back({ startIndex, index + 1 });
			if (isWhitePiece(m_board[index + 1]))
				break;
			index = index + 1;
		}
	}
	
}
void GameState::addRookRightToLeftMoves(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnLeftmostCol(index) && !isWhitePiece(m_board[index - 1])) {
			addMove(startIndex, index - 1);
			//av_moves.push_back({ startIndex, index - 1 });
			if (isBlackPiece(m_board[index - 1]))
				break;
			index = index - 1;
		}
	}
	else {
		while (!isOnLeftmostCol(index) && !isBlackPiece(m_board[index - 1])) {
			addMove(startIndex, index - 1);
			//av_moves.push_back({ startIndex, index - 1 });
			if (isWhitePiece(m_board[index - 1]))
				break;
			index = index - 1;
		}
	}
}

void GameState::addQueenMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'Q') {
			m_board[index] = 'B';
			addBishopMoves(index);
			m_board[index] = 'R';
			addRookMoves(index);
			m_board[index] = 'Q';
		}
	}
	else {
		if (m_board[index] == 'q') {
			m_board[index] = 'b';
			addBishopMoves(index);
			m_board[index] = 'r';
			addRookMoves(index);
			m_board[index] = 'q';
		}
	}
}

void GameState::addKingMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'K') {
			addKingMovesDown(index);
			addKingMovesUp(index);
			addKingMovesleft(index);
			addKingMovesRight(index);
			addKingMovesDownLeft(index);
			addKingMovesDownRight(index);
			addKingMovesUpLeft(index);
			addKingMovesUpRight(index);
			addKingCastlingMoves(index);
		}
	}
	else {
		if (m_board[index] == 'k') {
			addKingMovesDown(index);
			addKingMovesUp(index);
			addKingMovesleft(index);
			addKingMovesRight(index);
			addKingMovesDownLeft(index);
			addKingMovesDownRight(index);
			addKingMovesUpLeft(index);
			addKingMovesUpRight(index);
			addKingCastlingMoves(index);
		}
	}
	
}

void GameState::addKingMovesDown(int index) {
	if (m_turn) {
		if (isValid(index + rowLength) && !isWhitePiece(m_board[index + rowLength])
			&& !SquareIsAttackedByBlack(index + rowLength)) {
			addMove(index, index + rowLength);
			//av_moves.push_back({ index, index + rowLength });
		}
	}
	else {
		if (isValid(index + rowLength) && !isBlackPiece(m_board[index + rowLength])
			&& !SquareIsAttackedByWhite(index + rowLength)) {
			addMove(index, index + rowLength);
			//av_moves.push_back({ index, index + rowLength });
		}
	}
}

void GameState::addKingMovesUp(int index) {
	if (m_turn) {
		if (isValid(index - rowLength) && !isWhitePiece(m_board[index - rowLength])
			&& !SquareIsAttackedByBlack(index - rowLength)) {
			addMove(index, index - rowLength);
			//av_moves.push_back({ index, index - rowLength });
		}
	}
	else {
		if (isValid(index - rowLength) && !isBlackPiece(m_board[index - rowLength])
			&& !SquareIsAttackedByWhite(index - rowLength)) {
			addMove(index, index - rowLength);
			//av_moves.push_back({ index, index - rowLength });
		}
	}
}

void GameState::addKingMovesleft(int index) {
	if (m_turn) {
		if (!isOnLeftmostCol(index) && !isWhitePiece(m_board[index - 1])
			&& !SquareIsAttackedByBlack(index - 1)) {
			addMove(index, index - 1);
			//av_moves.push_back({ index, index - 1 });
		}
	}
	else {
		if (!isOnLeftmostCol(index) && !isBlackPiece(m_board[index - 1])
			&& !SquareIsAttackedByWhite(index - 1)) {
			addMove(index, index - 1);
			//av_moves.push_back({ index, index - 1 });
		}
	}
}

void GameState::addKingMovesRight(int index) {
	if (m_turn) {
		if (!isOnRightmostCol(index) && !isWhitePiece(m_board[index + 1])
			&& !SquareIsAttackedByBlack(index + 1)) {
			addMove(index, index + 1);
			//av_moves.push_back({ index, index + 1 });
		}
	}
	else {
		if (!isOnRightmostCol(index) && !isBlackPiece(m_board[index + 1])
			&& !SquareIsAttackedByWhite(index + 1)) {
			addMove(index, index + 1);
			//av_moves.push_back({ index, index + 1 });
		}
	}
}

void GameState::addKingMovesDownLeft(int index) {
	if (m_turn) {
		if (!isOnLeftmostCol(index) && isValid(index + rowLength)
			&& !isWhitePiece(m_board[index + rowLength - 1])
			&& !SquareIsAttackedByBlack(index + rowLength - 1)) 
		{
			addMove(index, index + rowLength - 1);
			//av_moves.push_back({ index, index + rowLength - 1});
		}
	}
	else {
		if (!isOnLeftmostCol(index) && isValid(index + rowLength)
			&& !isBlackPiece(m_board[index + rowLength - 1])
			&& !SquareIsAttackedByWhite(index + rowLength - 1))
		{
			addMove(index, index + rowLength - 1);
			//av_moves.push_back({ index, index + rowLength - 1 });
		}
	}
}

void GameState::addKingMovesDownRight(int index) {
	if (m_turn) {
		if (!isOnRightmostCol(index) && isValid(index + rowLength)
			&& !isWhitePiece(m_board[index + rowLength + 1])
			&& !SquareIsAttackedByBlack(index + rowLength + 1))
		{
			addMove(index, index + rowLength + 1);
			//av_moves.push_back({ index, index + rowLength + 1 });
		}
	}
	else {
		if (!isOnRightmostCol(index) && isValid(index + rowLength)
			&& !isBlackPiece(m_board[index + rowLength + 1])
			&& !SquareIsAttackedByWhite(index + rowLength + 1))
		{
			addMove(index, index + rowLength + 1);
			//av_moves.push_back({ index, index + rowLength + 1 });
		}
	}
}

void GameState::addKingMovesUpLeft(int index) {
	if (m_turn) {
		if (!isOnLeftmostCol(index) && isValid(index - rowLength)
			&& !isWhitePiece(m_board[index - rowLength - 1])
			&& !SquareIsAttackedByBlack(index - rowLength - 1))
		{
			addMove(index, index - rowLength - 1);
			//av_moves.push_back({ index, index - rowLength - 1 });
		}
	}
	else {
		if (!isOnLeftmostCol(index) && isValid(index - rowLength)
			&& !isBlackPiece(m_board[index - rowLength - 1])
			&& !SquareIsAttackedByWhite(index - rowLength - 1))
		{
			addMove(index, index - rowLength - 1);
			//av_moves.push_back({ index, index - rowLength - 1 });
		}
	}
	
}

void GameState::addKingMovesUpRight(int index) {
	if (m_turn) {
		if (!isOnRightmostCol(index) && isValid(index - rowLength)
			&& !isWhitePiece(m_board[index - rowLength + 1])
			&& !SquareIsAttackedByBlack(index - rowLength + 1))
		{
			addMove(index, index - rowLength + 1);
			//av_moves.push_back({ index, index - rowLength + 1 });
		}
	}
	else {
		if (!isOnRightmostCol(index) && isValid(index - rowLength)
			&& !isBlackPiece(m_board[index - rowLength + 1])
			&& !SquareIsAttackedByWhite(index - rowLength + 1))
		{
			addMove(index, index - rowLength + 1);
			//av_moves.push_back({ index, index - rowLength + 1 });
		}
	}
}

void GameState::addKingCastlingMoves(int index) {
	if (m_turn) {
		if (m_board[index] == 'K') {
			addKingMovesCastlingShort(index);
			addKingMovesCastlingLong(index);
		}
	}
	else {
		if (m_board[index] == 'k') {
			addKingMovesCastlingShort(index);
			addKingMovesCastlingLong(index);
		}
	}
}

void GameState::addKingMovesCastlingShort(int index) {
	if (m_turn) {
		if (m_shortCastleW && m_board[index - 1] == '-' && m_board[index - 2] == '-'
			&& !SquareIsAttackedByBlack(index-1) && !SquareIsAttackedByBlack(index - 2)
			&& !SquareIsAttackedByBlack(index))
		{
			av_moves.push_back({ index, index - 2 });
		}
	}
	else {
		if (m_shortCastleB && m_board[index - 1] == '-' && m_board[index - 2] == '-'
			&& !SquareIsAttackedByWhite(index - 1) && !SquareIsAttackedByWhite(index - 2)
			&& !SquareIsAttackedByWhite(index))
		{
			av_moves.push_back({ index, index - 2 });
		}

	}
}

void GameState::addKingMovesCastlingLong(int index) {
	if (m_turn) {
		if (m_longCastleW && m_board[index + 1] == '-' && m_board[index + 2] == '-'
			&& m_board[index + 3] == '-' && !SquareIsAttackedByBlack(index+1) && !SquareIsAttackedByBlack(index + 2)
			&& !SquareIsAttackedByBlack(index))
		{
			av_moves.push_back({ index, index + 2 });
		}
	}
	else {
		if (m_longCastleB && m_board[index + 1] == '-' && m_board[index + 2] == '-'
			&& m_board[index + 3] == '-' && !SquareIsAttackedByWhite(index + 1) && !SquareIsAttackedByWhite(index + 2)
			&& !SquareIsAttackedByWhite(index))
		{
			av_moves.push_back({ index, index + 2 });
		}

	}
}


void GameState::upLeftToDownRight(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnRightmostCol(index) && isValid(index + rowLength + 1)
			&& !isWhitePiece(m_board[index + rowLength + 1]))
		{
			addMove(startIndex, index + rowLength + 1);
			//av_moves.push_back({ startIndex, index + rowLength + 1 });
			if (isBlackPiece(m_board[index + rowLength + 1]))
				break;
			index += rowLength + 1;
		}
	}
	else {
		while (!isOnRightmostCol(index) && isValid(index + rowLength + 1)
			&& !isBlackPiece(m_board[index + rowLength + 1]))
		{
			addMove(startIndex, index + rowLength + 1);
			//av_moves.push_back({ startIndex, index + rowLength + 1 });
			if (isWhitePiece(m_board[index + rowLength + 1]))
				break;
			index += rowLength + 1;
		}
	}
	
}

void GameState::downRightToUpLeft(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnLeftmostCol(index) && isValid(index - rowLength - 1)
			&& !isWhitePiece(m_board[index - rowLength - 1]))
		{
			addMove(startIndex, index - rowLength - 1);
			//av_moves.push_back({ startIndex, index - rowLength - 1 });
			if (isBlackPiece(m_board[index - rowLength - 1]))
				break;
			index = index - rowLength - 1;
		}
	}
	else {
		while (!isOnLeftmostCol(index) && isValid(index - rowLength - 1)
			&& !isBlackPiece(m_board[index - rowLength - 1]))
		{
			addMove(startIndex, index - rowLength - 1);
			//av_moves.push_back({ startIndex, index - rowLength - 1 });
			if (isWhitePiece(m_board[index - rowLength - 1]))
				break;
			index = index - rowLength - 1;
		}
	}
}

void GameState::upRightToDownLeft(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnLeftmostCol(index) && isValid(index + rowLength)
			&& !isWhitePiece(m_board[index + rowLength - 1]))
		{
			addMove(startIndex, index + rowLength - 1);
			//av_moves.push_back({ startIndex, index + rowLength - 1 });
			if (isBlackPiece(m_board[index + rowLength - 1]))
				break;
			index = index + rowLength - 1;
		}
	}
	else {
		while (!isOnLeftmostCol(index) && isValid(index + rowLength)
			&& !isBlackPiece(m_board[index + rowLength - 1]))
		{
			addMove(startIndex, index + rowLength - 1);
			//av_moves.push_back({ startIndex, index + rowLength - 1 });
			if (isWhitePiece(m_board[index + rowLength - 1]))
				break;
			index = index + rowLength - 1;
		}
	}
}

void GameState::downLeftToUpRight(int index) {
	int startIndex{ index };
	if (m_turn) {
		while (!isOnRightmostCol(index) && isValid(index - rowLength)
			&& !isWhitePiece(m_board[index - rowLength + 1]))
		{
			addMove(startIndex, index - rowLength + 1);
			//av_moves.push_back({ startIndex, index - rowLength + 1 });
			if (isBlackPiece(m_board[index - rowLength + 1]))
				break;
			index = index - rowLength + 1;
		}
	}
	else {
		while (!isOnRightmostCol(index) && isValid(index - rowLength)
			&& !isBlackPiece(m_board[index - rowLength + 1]))
		{
			addMove(startIndex, index - rowLength + 1);
			//av_moves.push_back({ startIndex, index - rowLength + 1 });
			if (isWhitePiece(m_board[index - rowLength + 1]))
				break;
			index = index - rowLength + 1;
		}
	}
}

bool GameState::isOnRightmostCol(int index) {
	return index > rowLength - 2 && (index + 1) % rowLength == 0;
}

bool GameState::isOnLeftmostCol(int index) {
	return index == 0 || index % 8 == 0;
}

bool GameState::isValid(int index) {
	return index > -1 && index < m_board.size();
}

bool GameState::isBlackPiece(char piece) {
	if (piece == '-')
		return false;
	for (char c : blackPieces) {
		if (piece == c) {
			return true;
		}
	}
	return false;
}

bool GameState::isWhitePiece(char piece) {
	if (piece == '-')
		return false;
	for (char c : whitePieces) {
		if (piece == c) {
			return true;
		}
	}
	return false;
}

bool GameState::isBlackPiece(int index) {
	return isBlackPiece(m_board[index]);
}

bool GameState::isWhitePiece(int index) {
	return isWhitePiece(m_board[index]);
}

bool GameState::SquareIsAttackedByWhite(int index) {
	return attackedDiagonallyByWhite(index)
		|| attackedByWhiteKing(index)
		|| attackedByWhiteHorizontally(index)
		|| attackedByWhiteVertically(index)
		|| attackedByWhiteKnight(index);
}

bool GameState::attackedDiagonallyByWhite(int index) {
	return attackedByWhitePawn(index)
		|| attackedByWhiteRightDownToUpLeft(index)
		|| attackedByWhiteLeftDownToUpRight(index)
		|| attackedByWhiteUpLeftDownRight(index)
		|| attackedByWhiteUpRightDownLeft(index);
}

bool GameState::SquareIsAttackedByBlack(int index) {
	return attackedDiagonallyByBlack(index)
		|| attackedByBlackKing(index)
		|| attackedByBlackHorizontally(index)
		|| attackedByBlackVertically(index)
		|| attackedByBlackKnight(index);
}

/* Start - Attacked diagonally */

bool GameState::attackedDiagonallyByBlack(int index) {
	return attackedByBlackPawn(index)
		|| attackedByBlackRightDownToUpLeft(index)
		|| attackedByBlackLeftDownToUpRight(index)
		|| attackedByBlackUpLeftDownRight(index)
		|| attackedByBlackUpRightDownLeft(index);
}

bool GameState::attackedByWhiteRightDownToUpLeft(int index) {
	while (true) {
		if (!isValid(index + rowLength) || isOnRightmostCol(index))
			return false;
		if (isBlackPiece(m_board[index + rowLength + 1]))
			return false;
		if (m_board[index + rowLength + 1] == 'B' || m_board[index + rowLength + 1] == 'Q') {
			return true;
		}
		if (isWhitePiece(m_board[index + rowLength + 1]))
			return false;
		index = index + rowLength + 1;
	}
}

bool GameState::attackedByWhiteLeftDownToUpRight(int index) {
	while (true) {
		if (!isValid(index + rowLength) || isOnLeftmostCol(index))
			return false;
		if (isBlackPiece(m_board[index + rowLength - 1]))
			return false;
		if (m_board[index + rowLength - 1] == 'B' || m_board[index + rowLength - 1] == 'Q') {
			return true;
		}
		if (isWhitePiece(m_board[index + rowLength - 1]))
			return false;
		index = index + rowLength - 1;
	}
}

bool GameState::attackedByWhiteUpLeftDownRight(int index) {
	while (true) {
		if (!isValid(index - rowLength) || isOnLeftmostCol(index))
			return false;
		if (isBlackPiece(m_board[index - rowLength - 1]))
			return false;
		if (m_board[index - rowLength - 1] == 'B' || m_board[index - rowLength - 1] == 'Q') {
			return true;
		}
		if (isWhitePiece(m_board[index - rowLength - 1]))
			return false;
		index = index - rowLength - 1;
	}
}

bool GameState::attackedByWhiteUpRightDownLeft(int index) {
	while (true) {
		if (!isValid(index - rowLength) || isOnRightmostCol(index))
			return false;
		if (isBlackPiece(m_board[index - rowLength + 1]))
			return false;
		if (m_board[index - rowLength + 1] == 'B' || m_board[index - rowLength + 1] == 'Q') {
			return true;
		}
		if (isWhitePiece(m_board[index - rowLength + 1]))
			return false;
		index = index - rowLength + 1;
	}
}

bool GameState::attackedByWhitePawn(int index) {
	if (!isValid(index - 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index - rowLength - 1] == 'P')
		return true;
	if (!isOnRightmostCol(index) && m_board[index - rowLength + 1] == 'P')
		return true;
	return false;
}

bool GameState::attackedByWhiteKing(int index) {
	if (!isOnLeftmostCol(index) && m_board[index - 1] == 'K')
		return true;
	if (!isOnRightmostCol(index) && m_board[index + 1] == 'K')
		return true;
	if (isValid(index + rowLength) && m_board[index + rowLength] == 'K')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength] == 'K')
		return true;
	if (isValid(index + rowLength) && !isOnLeftmostCol(index) && m_board[index + rowLength - 1] == 'K')
		return true;
	if (isValid(index + rowLength) && !isOnRightmostCol(index) && m_board[index + rowLength + 1] == 'K')
		return true;
	if (isValid(index - rowLength) && !isOnLeftmostCol(index) && m_board[index - rowLength - 1] == 'K')
		return true;
	if (isValid(index - rowLength) && !isOnRightmostCol(index) && m_board[index - rowLength + 1] == 'K')
		return true;
	return false;
}

// ------------------------------------------------------------------

bool GameState::attackedByBlackRightDownToUpLeft(int index) {
	while (true) {
		if (!isValid(index + rowLength) || isOnRightmostCol(index))
			return false;
		if (isWhitePiece(m_board[index + rowLength + 1]))
			return false;
		if (m_board[index + rowLength + 1] == 'b' || m_board[index + rowLength + 1] == 'q') {
			return true;
		}
		if (isBlackPiece(m_board[index + rowLength + 1]))
			return false;
		index = index + rowLength + 1;
	}
}

bool GameState::attackedByBlackLeftDownToUpRight(int index) {
	while (true) {
		if (!isValid(index + rowLength) || isOnLeftmostCol(index))
			return false;
		if (isWhitePiece(m_board[index + rowLength - 1]))
			return false;
		if (m_board[index + rowLength - 1] == 'b' || m_board[index + rowLength - 1] == 'q') {
			return true;
		}
		if (isBlackPiece(m_board[index + rowLength - 1]))
			return false;
		index = index + rowLength - 1;
	}
}

bool GameState::attackedByBlackUpLeftDownRight(int index) {
	while (true) {
		if (!isValid(index - rowLength) || isOnLeftmostCol(index))
			return false;
		if (isWhitePiece(m_board[index - rowLength - 1]))
			return false;
		if (m_board[index - rowLength - 1] == 'b' || m_board[index - rowLength - 1] == 'q') {
			return true;
		}
		if (isBlackPiece(m_board[index - rowLength - 1]))
			return false;
		index = index - rowLength - 1;
	}
}

bool GameState::attackedByBlackUpRightDownLeft(int index) {
	while (true) {
		if (!isValid(index - rowLength) || isOnRightmostCol(index))
			return false;
		if (isWhitePiece(m_board[index - rowLength + 1]))
			return false;
		if (m_board[index - rowLength + 1] == 'b' || m_board[index - rowLength + 1] == 'q') {
			return true;
		}
		if (isBlackPiece(m_board[index - rowLength + 1]))
			return false;
		index = index - rowLength + 1;
	}
}

bool GameState::attackedByBlackPawn(int index) {
	if (!isValid(index + 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index + rowLength - 1] == 'p')
		return true;
	if (!isOnRightmostCol(index) && m_board[index + rowLength + 1] == 'p')
		return true;
	return false;
}

bool GameState::attackedByBlackKing(int index) {
	if (!isOnLeftmostCol(index) && m_board[index - 1] == 'k')
		return true;
	if (!isOnRightmostCol(index) && m_board[index + 1] == 'k')
		return true;
	if (isValid(index + rowLength) && m_board[index + rowLength] == 'k')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength] == 'k')
		return true;
	if (isValid(index + rowLength) && !isOnLeftmostCol(index) && m_board[index + rowLength - 1] == 'k')
		return true;
	if (isValid(index + rowLength) && !isOnRightmostCol(index) && m_board[index + rowLength + 1] == 'k')
		return true;
	if (isValid(index - rowLength) && !isOnLeftmostCol(index) && m_board[index - rowLength - 1] == 'k')
		return true;
	if (isValid(index - rowLength) && !isOnRightmostCol(index) && m_board[index - rowLength + 1] == 'k')
		return true;
	return false;
}

/* End - Attacked diagonally */

/* Start - Attacked horizontally */

bool GameState::attackedByWhiteHorizontally(int index) {
	return attackedByWhiteHorizontallyFromLeft(index)
		|| attackedByWhiteHorizontallyFromRight(index);
}

bool GameState::attackedByWhiteHorizontallyFromLeft(int index) {
	while (true) {
		if (isOnLeftmostCol(index))
			return false;
		if (isBlackPiece(m_board[index - 1]))
			return false;
		if (m_board[index - 1] == 'R' || m_board[index - 1] == 'Q')
			return true;
		if (isWhitePiece(m_board[index - 1]))
			return false;
		index = index - 1;
	}
}

bool GameState::attackedByWhiteHorizontallyFromRight(int index) {
	while (true) {
		if (isOnRightmostCol(index))
			return false;
		if (isBlackPiece(m_board[index + 1]))
			return false;
		if (m_board[index + 1] == 'R' || m_board[index + 1] == 'Q')
			return true;
		if (isWhitePiece(m_board[index + 1]))
			return false;
		index = index + 1;
	}
}

//---------------------------------------------------------

bool GameState::attackedByBlackHorizontally(int index) {
	return attackedByBlackHorizontallyFromLeft(index)
		|| attackedByBlackHorizontallyFromRight(index);
}

bool GameState::attackedByBlackHorizontallyFromLeft(int index) {
	while (true) {
		if (isOnLeftmostCol(index))
			return false;
		if (isWhitePiece(m_board[index - 1]))
			return false;
		if (m_board[index - 1] == 'r' || m_board[index - 1] == 'q')
			return true;
		if (isBlackPiece(m_board[index - 1]))
			return false;
		index = index - 1;
	}
}

bool GameState::attackedByBlackHorizontallyFromRight(int index) {
	while (true) {
		if (isOnRightmostCol(index))
			return false;
		if (isWhitePiece(m_board[index + 1]))
			return false;
		if (m_board[index + 1] == 'r' || m_board[index + 1] == 'q')
			return true;
		if (isBlackPiece(m_board[index + 1]))
			return false;
		index = index + 1;
	}
}

/* End - Attacked horizontally */

/* Start - Attacked vertically */

bool GameState::attackedByWhiteVertically(int index) {
	return attackedByWhiteVerticallyFromUp(index)
		|| attackedByWhiteVerticallyFromDown(index);
}

bool GameState::attackedByWhiteVerticallyFromUp(int index)
{
	while (true) {
		if (!isValid(index - rowLength))
			return false;
		if (isBlackPiece(index - rowLength))
			return false;
		if (m_board[index - rowLength] == 'Q' || m_board[index - rowLength] == 'R')
			return true;
		if (isWhitePiece(index - rowLength))
			return false;
		index = index - rowLength;
	}
}

bool GameState::attackedByWhiteVerticallyFromDown(int index) {
	while (true) {
		if (!isValid(index + rowLength))
			return false;
		if (isBlackPiece(index + rowLength))
			return false;
		if (m_board[index + rowLength] == 'Q' || m_board[index + rowLength] == 'R')
			return true;
		if (isWhitePiece(index + rowLength))
			return false;
		index = index + rowLength;
	}
}

//-----------------------------------------------------------------------------

bool GameState::attackedByBlackVertically(int index) {
	return attackedByBlackVerticallyFromUp(index)
		|| attackedByBlackVerticallyFromDown(index);
}

bool GameState::attackedByBlackVerticallyFromUp(int index)
{
	while (true) {
		if (!isValid(index - rowLength))
			return false;
		if (isWhitePiece(index - rowLength))
			return false;
		if (m_board[index - rowLength] == 'q' || m_board[index - rowLength] == 'r')
			return true;
		if (isBlackPiece(index - rowLength))
			return false;
		index = index - rowLength;
	}
}

bool GameState::attackedByBlackVerticallyFromDown(int index) {
	while (true) {
		if (!isValid(index + rowLength))
			return false;
		if (isWhitePiece(index + rowLength))
			return false;
		if (m_board[index + rowLength] == 'q' || m_board[index + rowLength] == 'r')
			return true;
		if (isBlackPiece(index + rowLength))
			return false;
		index = index + rowLength;
	}
}

/* End - Attacked vertically */

/* Start - Attacked by knight */

bool GameState::attackedByWhiteKnight(int index) {
	return attackedByWhiteKnightVerticallyFromUp(index)
		|| attackedByWhiteKnightVerticallyFromDown(index)
		|| attackedByWhiteKnightHorizontally(index);
}

bool GameState::attackedByWhiteKnightVerticallyFromUp(int index) {
	if (!isValid(index - 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index - 2 * rowLength - 1] == 'N')
		return true;
	if (!isOnRightmostCol(index) && m_board[index - 2 * rowLength + 1] == 'N')
		return true;
	return false;
}

bool GameState::attackedByWhiteKnightVerticallyFromDown(int index) {
	if (!isValid(index + 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index + 2 * rowLength - 1] == 'N')
		return true;
	if (!isOnRightmostCol(index) && m_board[index + 2 * rowLength + 1] == 'N')
		return true;
	return false;
}

bool GameState::attackedByWhiteKnightHorizontally(int index) {
	return attackedByWhiteKnightHorizontallyFromLeft(index)
		|| attackedByWhiteKnightHorizontallyFromRight(index);
}

bool GameState::attackedByWhiteKnightHorizontallyFromLeft(int index) {
	if (isOnLeftmostCol(index) || isOnLeftmostCol(index - 1))
		return false;
	if (isValid(index + rowLength) && m_board[index + rowLength - 2] == 'N')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength - 2] == 'N')
		return true;
	return false;
}

bool GameState::attackedByWhiteKnightHorizontallyFromRight(int index) {
	if (isOnRightmostCol(index) || isOnRightmostCol(index + 1))
		return false;
	if (isValid(index + rowLength) && m_board[index + rowLength + 2] == 'N')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength + 2] == 'N')
		return true;
	return false;
}

// ------------------------------------------------------------

bool GameState::attackedByBlackKnight(int index) {
	return attackedByBlackKnightVerticallyFromUp(index)
		|| attackedByBlackKnightVerticallyFromDown(index)
		|| attackedByBlackKnightHorizontally(index);
}

bool GameState::attackedByBlackKnightVerticallyFromUp(int index) {
	if (!isValid(index - 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index - 2 * rowLength - 1] == 'n')
		return true;
	if (!isOnRightmostCol(index) && m_board[index - 2 * rowLength + 1] == 'n')
		return true;
	return false;
}

bool GameState::attackedByBlackKnightVerticallyFromDown(int index) {
	if (!isValid(index + 2 * rowLength))
		return false;
	if (!isOnLeftmostCol(index) && m_board[index + 2 * rowLength - 1] == 'n')
		return true;
	if (!isOnRightmostCol(index) && m_board[index + 2 * rowLength + 1] == 'n')
		return true;
	return false;
}


bool GameState::attackedByBlackKnightHorizontally(int index) {
	return attackedByBlackKnightHorizontallyFromLeft(index)
		|| attackedByBlackKnightHorizontallyFromRight(index);
}

bool GameState::attackedByBlackKnightHorizontallyFromLeft(int index) {
	if (isOnLeftmostCol(index) || isOnLeftmostCol(index - 1))
		return false;
	if (isValid(index + rowLength) && m_board[index + rowLength - 2] == 'n')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength - 2] == 'n')
		return true;
	return false;
}

bool GameState::attackedByBlackKnightHorizontallyFromRight(int index) {
	if (isOnRightmostCol(index) || isOnRightmostCol(index + 1))
		return false;
	if (isValid(index + rowLength) && m_board[index + rowLength + 2] == 'n')
		return true;
	if (isValid(index - rowLength) && m_board[index - rowLength + 2] == 'n')
		return true;
	return false;
}



// -------------------------------------------------------------------------------

double GameState::eval() {
	if (gameOver()) {
		if (isCheckmate()) {
			if (m_turn)
				return -10000;
			else
				return 10000;
		}
		else {
			return 0;
		}
	}
	double score{ 0 };
	int index{ 0 };
	for (char piece : m_board) {
		if (piece == '-' || piece == 'K' || piece == 'k')
			continue;
		if (piece == 'P')
		{
			++score;
		}
		else if (piece == 'p') {
			--score;
		}
		else if (piece == 'Q')
			score += 9;
		else if (piece == 'q')
			score -= 9;
		else if (piece == 'R') {
			score += 5;
		}
		else if (piece == 'r')
			score -= 5;
		else if (piece == 'B') {
			score += 3;
		}
		else if (piece == 'b')
		{
			score -= 3;
		}
		else if (piece == 'N') {
			score += 2.8;
		}
			
		else if (piece == 'n') {
			score -= 2.8;
		}
		if(isWhitePiece(piece) && !isValid(index-rowLength)){
			score -= 0.2;
		}
		else if (isBlackPiece(piece) && !isValid(index + rowLength)) {
			score += 0.2;
		}
		++index;
	}
	return score;
}

MoveScore GameState::computerMove(int depth, double a, double b) {
	if (gameOver()) {
		++perftPositions;
		return MoveScore{ {-1,-1}, eval() };
	}
	
	else if(depth >= 4) {
		++perftPositions;
		return MoveScore{ {-1,-1}, eval() };
	}
	double score{m_turn? -100000.0 : 100000.0};
	std::pair<int, int> m{ -1,-1 };
	MoveScore moveScor{ {-1,-1}, 0 };
	getAvMoves();
	std::vector<std::pair<int, int>> availables{ av_moves };
	//std::sort(availables.begin(), availables.end(),
	//	[*this](std::pair<int, int>& f, std::pair<int, int>& s) mutable
	//{
	//	return (m_board[f.second] == '-' && m_board[s.second] != '-');
	//	/*|| (!kingIsAttacked(f) && kingIsAttacked(s));*/
	//}); <-- This is actually slower!
	for (auto& move : availables) {
		if (m.first == -1) {
			m = move;
		}
		newChessMove(move.first, move.second, *this);
		playMove();
		moveScor = computerMove(depth + 1, a , b);
		undoMove();
		if (m_turn) {
			if (moveScor.score > score) {
				score = moveScor.score;
				m = move;
			}
			if (score > a) {
				a = score;
			}
			if (b <= a)
				break;
		}
		else {
			if (moveScor.score < score) {
				score = moveScor.score;
				m = move;
			}
			if (score < b) {
				b = score;
			}
			if (b <= a)
				break;
		}
		
	}
	return { m, score };
}

bool GameState::kingIsAttacked(const std::pair<int, int>& move){
	newChessMove(move.first, move.second, *this);
	bool attacked{ false };
	playMove();
	if (m_turn) {
		attacked = SquareIsAttackedByBlack(whiteKingIndex);
	}
	else {
		attacked = SquareIsAttackedByWhite(blackKingIndex);
	}
	undoMove();
	return attacked;
}

bool GameState::kingIsAttacked() {
	if (m_turn) {
		return SquareIsAttackedByBlack(whiteKingIndex);
	}
	else {
		return SquareIsAttackedByWhite(blackKingIndex);
	}
}

bool GameState::hasGoodCapture() {
	getAvMoves();
	if (m_turn) {
		for (auto& move : av_moves) {
			if (m_board[move.second] == '-')
				continue;
			if (!SquareIsAttackedByBlack(move.second))
				return true;
			if (map[m_board[move.second]] >= map[m_board[move.first]])
				return true;
		}
	}
	else {
		for (auto& move : av_moves) {
			if (m_board[move.second] == '-')
				continue;
			if (!SquareIsAttackedByWhite(move.second))
				return true;
			if (map[m_board[move.second]] >= map[m_board[move.first]])
				return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------













//MoveScore GameState::computerMove(int depth, double a, double b) {
//	if (gameOver()) {
//		return MoveScore{ {-1,-1}, eval() };
//	}
//	else if (depth >= 6) {
//		return MoveScore{ {-1,-1}, eval() };
//	}
//	double score{ m_turn ? -100000.0 : 100000.0 };
//	std::pair<int, int> m{ -1,-1 };
//	MoveScore moveScor{ {-1,-1}, 0 };
//	getAvMoves();
//	std::vector<std::pair<int, int>> avs{ av_moves };
//	std::vector<std::pair<int, int>> availables{};
//	std::vector<MoveScore> scores(avs.size());
//	int index{ 0 };
//	for (auto& move : avs) {
//		newChessMove(move.first, move.second, *this);
//		playMove();
//		scores[index++] = { move, eval() };
//		undoMove();
//	}
//	int maxMoves{ scores.size() >= 10 ? 10 : static_cast<int>(scores.size()) };
//	std::sort(scores.begin(), scores.end(), [](MoveScore& f, MoveScore& s) {return f.score > s.score; });
//	for (int i = 0; i < maxMoves; ++i) {
//		availables.push_back(scores[i].move);
//	}
//
//	/*if (availables.size() <= 10)
//		--depth;*/
//		//int equalMoves{ 1 };
//	for (auto& move : availables) {
//		//newChessMove(startIndex, endIndex, *this);
//		if (m.first == -1) {
//			m = move;
//		}
//		newChessMove(move.first, move.second, *this);
//		//if(move.first )
//		playMove();
//		moveScor = computerMove(depth + 1, a, b);
//		undoMove();
//		/*if (moveScor.score == score) {
//			++equalMoves;
//			int choice{ Random::get(1,equalMoves) };
//			if (choice == 1) {
//				m = move;
//			}
//		}*/
//		if (m_turn) {
//			if (moveScor.score > score) {
//				score = moveScor.score;
//				m = move;
//			}
//			if (score > a) {
//				a = score;
//			}
//			if (b <= a)
//				break;
//		}
//		else {
//			if (moveScor.score < score) {
//				score = moveScor.score;
//				m = move;
//			}
//			if (score < b) {
//				b = score;
//			}
//			if (b <= a)
//				break;
//		}
//	}
//	return { m, score };
//}
