#pragma once
#include "BoardState.h"
#include "Square.h"
#include <string>
#include <string_view>
class BoardState;
class ChessMove
{

public:
	enum Type {
		normal,
		capture,
		shortCastling,
		longCastling,
		enPasant,
		promotion,
		king,
		rook
	};
	
	ChessMove(const Square& start, const Square& end, const BoardState& board);

	bool isPromotion();

	bool find(std::string_view s, char c);

	void getPromotionPiece();

	void setType(const BoardState& board, bool compMove=true);

	ChessMove(int startIndex, int endIndex, const BoardState& board);

	void newChessMove(const Square& start, const Square& end, const BoardState& board, bool compMove=true);

	void newChessMove(int startIndex, int endIndex, const BoardState& board, bool compMove=true);

	void newChessMove(const ChessMove& move);

	void getSquare(int index, Square& square, const BoardState& board);
	
	ChessMove() {};

	int getIndex(int row, int col, int rowLength=8) const;
	bool isCaptureMove(const BoardState& board);

	bool isShortCastling(const BoardState& board);

	bool isLongCastling(const BoardState& board);

	bool isEnPasant(const BoardState& board);

	void playMove(BoardState& board);

	// --------------------------------------------------

	Square m_startSquare;
	Square m_destSquare;
	int m_startIndex{};
	int m_endIndex{};

	char charAtStart{};
	char charAtEnd{};
	char enPasantCatpuredPiece{};
	int enPasantIndex{};

	char promotionPiece{};

	std::string_view promotions{ "qrbn" };

	bool onMove{};

	Type type{};
	

};

