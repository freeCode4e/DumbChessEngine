#include "ChessMove.h"
#include <iostream>
class BoardState;

ChessMove::ChessMove(const Square& start, const Square& end, const BoardState& board) :
	m_startSquare{ start }, m_destSquare{ end }, onMove{ board.m_turn },
	charAtStart{ board.m_board[getIndex(start.row, start.col, 8)] },
	charAtEnd{ board.m_board[getIndex(end.row, end.col, 8)] },
	m_startIndex{ getIndex(start.row, start.col) },
	m_endIndex{ getIndex(end.row, end.col) },
	enPasantCatpuredPiece{ '-' }, enPasantIndex{ -1 }
{
	setType(board);
}

bool ChessMove::isPromotion() {
	return ((charAtStart == 'P' || charAtStart == 'p') && (m_endIndex >= 56 || m_endIndex < 8));
}

bool ChessMove::find(std::string_view s, char c) {
	return std::find(s.begin(), s.end(), c) != s.end();
}

void ChessMove::getPromotionPiece() {
	char p{};
	while (true) {
		std::cout << "Promotion piece: ";
		std::cin >> p;
		if (find(promotions, p))
			break;
		else
			std::cout << "Invalid promotion. valid Promotions {q,r,b,n}\n";
	}
	promotionPiece = static_cast<char>((onMove ? std::toupper(p) : std::tolower(p)));
}

void ChessMove::setType(const BoardState& board, bool compMove) {
	if (isShortCastling(board)) {
		type = Type::shortCastling;
	}
	else if (isLongCastling(board)) {
		type = Type::longCastling;
	}

	else if (charAtStart == 'K' || charAtStart == 'k') {
		type = Type::king;
	}
	else if (charAtStart == 'R' || charAtStart == 'r') {
		type = Type::rook;
	}
	else if (isPromotion()) {
		type = Type::promotion;
		if (compMove)
			promotionPiece = (onMove ? 'Q' : 'q');
		else
			getPromotionPiece();
	}
	else if (isEnPasant(board)) {
		type = Type::enPasant;
		int diff{ std::abs(m_endIndex - m_startIndex) };
		// the condition is essentially the direction of en pasant
		if (diff > board.rowLength) {
			int nextTo{ onMove ? 1 : -1 };
			enPasantIndex = m_startIndex + nextTo;
			enPasantCatpuredPiece = board.m_board[enPasantIndex];
		}
		else {
			int nextTo{ onMove ? -1 : 1 };
			enPasantIndex = m_startIndex + nextTo;
			enPasantCatpuredPiece = board.m_board[enPasantIndex];
		}
	}
	else if (isCaptureMove(board)) {
		type = Type::capture;
	}
	else {
		type = Type::normal;
	}
}

ChessMove::ChessMove(int startIndex, int endIndex, const BoardState& board) {
	newChessMove(startIndex, endIndex, board);
}

void ChessMove::newChessMove(const Square& start, const Square& end, const BoardState& board, bool compMove) {
	m_startSquare = start;
	m_destSquare = end;
	onMove = board.m_turn;
	charAtStart = board.m_board[getIndex(start.row, start.col, 8)];
	charAtEnd = board.m_board[getIndex(end.row, end.col, 8)];
	m_startIndex = getIndex(start.row, start.col);
	m_endIndex = getIndex(end.row, end.col);
	enPasantCatpuredPiece = '-' ;
	enPasantIndex = -1;
	setType(board, compMove);
}

void ChessMove::newChessMove(int startIndex, int endIndex, const BoardState& board, bool compMove) {
	// index = row * rowLength + col, flattening 2d array (we want the reverse in this case)
	/*int row{ startIndex / board.rowLength };
	int col{ startIndex - row * board.rowLength };
	const Square start{ row, col };

	row = endIndex / board.rowLength;
	col = endIndex - row * board.rowLength;
	const Square end{}*/
	Square start{};
	Square end{};
	getSquare(startIndex, start, board);
	getSquare(endIndex, end, board);
	newChessMove(start, end, board, compMove);
}

void ChessMove::newChessMove(const ChessMove& move) {
	*this = move;
}

void ChessMove::getSquare(int index, Square& square, const BoardState& board) {
	square.row = index / board.rowLength;
	square.col = index - square.row * board.rowLength;
}

int ChessMove::getIndex(int row, int col, int rowLength) const {
	return row * rowLength + col;
}



bool ChessMove::isCaptureMove(const BoardState& board) {
	return (board.m_board[m_endIndex] != '-');
}

bool ChessMove::isShortCastling(const BoardState& board) {
	int startIndex{ getIndex(m_startSquare.row, m_startSquare.col) };
	int endIndex{ getIndex(m_destSquare.row, m_destSquare.col) };
	return (board.m_board[startIndex] == 'K' && endIndex == startIndex - 2
		|| board.m_board[startIndex] == 'k' && endIndex == startIndex - 2);
}

bool ChessMove::isLongCastling(const BoardState& board) {
	int startIndex{ getIndex(m_startSquare.row, m_startSquare.col) };
	int endIndex{ getIndex(m_destSquare.row, m_destSquare.col) };
	return (board.m_board[startIndex] == 'K' && endIndex == startIndex + 2
		|| board.m_board[startIndex] == 'k' && endIndex == startIndex + 2);
}

bool ChessMove::isEnPasant(const BoardState& board) {
	int startIndex{ getIndex(m_startSquare.row, m_startSquare.col) };
	int endIndex{ getIndex(m_destSquare.row, m_destSquare.col) };
	return ((board.m_board[startIndex] == 'P' || board.m_board[startIndex] == 'p')
		&& board.m_board[endIndex] == '-' && 
		std::abs(endIndex-startIndex) % board.rowLength != 0);
}

void ChessMove::playMove(BoardState& board) {
	// index = row * rowLength + col, flattening 2d array
	int startIndex{ m_startIndex };
	int endIndex{ m_endIndex };

	if (type == shortCastling) {
		int nextTo{ 1 };
		if (onMove == true)
			board.m_shortCastleW = false;
		else {
			board.m_shortCastleB = false;
			nextTo = -1;
		}
		board.m_board[endIndex] = board.m_board[startIndex];
		board.m_board[startIndex] = '-';
		board.m_board[startIndex + nextTo] = board.m_board[endIndex + nextTo];
		board.m_board[endIndex + nextTo] = '-';
	}
	else if (type == longCastling) {
		int nextTo{ -1 };
		if (onMove == true)
			board.m_longCastleW = false;
		else {
			board.m_longCastleB = false;
			nextTo = 1;
		}
		board.m_board[endIndex] = board.m_board[startIndex];
		board.m_board[startIndex] = '-';

		board.m_board[startIndex + nextTo] = board.m_board[endIndex + 2 * nextTo];
		board.m_board[endIndex + 2 * nextTo] = '-';
	}
	else if (type == enPasant) {
		board.m_board[endIndex] = board.m_board[startIndex];
		board.m_board[startIndex] = '-';

		board.m_board[enPasantIndex] = '-';
	}

	else {
		board.m_board[endIndex] = board.m_board[startIndex];
		board.m_board[startIndex] = '-';
	}
	board.m_turn = !board.m_turn;
}

//void ChessMove::playMove(const BoardState& board) {
//	// index = row * rowLength + col, flattening 2d array
//	int startIndex{ getIndex(startSquare.row, startSquare.col) };
//	int endIndex{ getIndex(destSquare.row, destSquare.col) };
//
//	 // ??? Move this later into minimax so that we don't keep capture when
//	if (isCapture(move)) {
//		// keep the captured piece so we can undo a move
//		m_captures.push(m_board[endIndex]);
//	}
//	if (isShortCastling(move)) {
//		// shift the rook over
//		m_board[startIndex + 1] = m_board[endIndex + 1];
//		m_board[endIndex + 1] = '-';
//		if (m_turn) {
//			m_shortCastleW = false;
//		}
//		else {
//			m_shortCastleB = false;
//		}
//	}
//	
//	m_board[endIndex] = m_board[startIndex];
//	m_board[startIndex] = '-';
//	m_turn = !m_turn;
//}


