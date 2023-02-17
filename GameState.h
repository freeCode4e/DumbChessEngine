#pragma once
#include "BoardState.h"
#include "ChessMove.h"
#include "MoveScore.h"
#include <stack>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <array>
#include <utility>
class GameState : public BoardState, public ChessMove
{
public:
	GameState();
	void tryMove(int startIndex, int endIndex);
	void playMove();
	void undoMove();
	void playMove(ChessMove& move);
	void playGame();
	bool gameOver();
	bool isCheckmate();
	bool isStalemate();
	bool isThreFold();
	bool insuficientMaterial();
	void getMove();
	int convertLetterToCol(char letter);
	int convertNumberToRow(char number);
	void convert(std::string_view move, int& startRow, int& startCol, int& endRow, int& endCol);
	bool isValidMove(std::string_view move);
	void getSquares(int& startRow, int& startCol, int& endRow, int& endCol);

	void getAvMoves();

	bool isAvMove(const std::pair<int,int>& move);

	void printAvMoves();

	void addMove(int startIndex, int endIndex);

	void addPownCaptures(int index);

	void downRightToUpLeft(int index);

	void upRightToDownLeft(int index);

	void downLeftToUpRight(int index);

	bool isOnRightmostCol(int index);

	bool isOnLeftmostCol(int index);

	void addEnpasant(int index);

	void addPawnMoves(int index);

	void addKnightMoves(int index);

	void addKnightVerticalMove(int index);

	void addKnightHorizontalMove(int index);

	void addBishopMoves(int index);

	void addRookMoves(int index);

	void addRookVerticalDownMoves(int index);

	void addRookVerticalUpMoves(int index);

	void addRookLeftToRightMoves(int index);

	void addRookRightToLeftMoves(int index);

	void addQueenMoves(int index);

	void addKingMoves(int index);

	void addKingMovesDown(int index);

	void addKingMovesUp(int index);

	void addKingMovesleft(int index);

	void addKingMovesRight(int index);

	void addKingMovesDownLeft(int index);

	void addKingMovesDownRight(int index);

	void addKingMovesUpLeft(int index);

	void addKingMovesUpRight(int index);

	void addKingCastlingMoves(int index);

	void addKingMovesCastlingShort(int index);

	void addKingMovesCastlingLong(int index);

	void upLeftToDownRight(int index);

	bool isValid(int index);

	bool isBlackPiece(char piece);

	bool isWhitePiece(char piece);

	bool isBlackPiece(int index);

	bool isWhitePiece(int index);

	bool SquareIsAttackedByWhite(int index);

	bool attackedDiagonallyByWhite(int index);

	bool SquareIsAttackedByBlack(int index);

	bool attackedDiagonallyByBlack(int index);

	bool attackedByWhiteRightDownToUpLeft(int index);

	bool attackedByWhiteLeftDownToUpRight(int index);

	bool attackedByWhiteUpLeftDownRight(int index);

	bool attackedByWhiteUpRightDownLeft(int index);

	bool attackedByWhitePawn(int index);

	bool attackedByWhiteKing(int index);

	bool attackedByBlackRightDownToUpLeft(int index);

	bool attackedByBlackLeftDownToUpRight(int index);

	bool attackedByBlackUpLeftDownRight(int index);

	bool attackedByBlackUpRightDownLeft(int index);

	bool attackedByBlackPawn(int index);

	bool attackedByBlackKing(int index);

	bool attackedByWhiteHorizontally(int index);

	bool attackedByWhiteHorizontallyFromLeft(int index);

	bool attackedByWhiteHorizontallyFromRight(int index);

	bool attackedByBlackHorizontally(int index);

	bool attackedByBlackHorizontallyFromLeft(int index);

	bool attackedByBlackHorizontallyFromRight(int index);

	bool attackedByWhiteVertically(int index);

	bool attackedByBlackVertically(int index);

	bool attackedByBlackVerticallyFromUp(int index);

	bool attackedByBlackVerticallyFromDown(int index);

	bool attackedByWhiteKnight(int index);

	bool attackedByWhiteKnightVerticallyFromUp(int index);

	bool attackedByWhiteKnightVerticallyFromDown(int index);

	bool attackedByWhiteKnightHorizontally(int index);

	bool attackedByWhiteKnightHorizontallyFromRight(int index);

	bool attackedByWhiteKnightHorizontallyFromLeft(int index);

	bool attackedByBlackKnight(int index);

	bool attackedByBlackKnightVerticallyFromUp(int index);

	bool attackedByBlackKnightVerticallyFromDown(int index);

	bool attackedByBlackKnightHorizontally(int index);

	bool attackedByBlackKnightHorizontallyFromLeft(int index);

	bool attackedByBlackKnightHorizontallyFromRight(int index);

	double eval();

	MoveScore computerMove(int depth=0, double a= -999'999.0, double b= 999'999.0);

	bool kingIsAttacked(const std::pair<int, int>& move);

	bool kingIsAttacked();

	bool hasGoodCapture();


	bool attackedByKnight(int index);

	bool attackedByWhiteVerticallyFromUp(int index);

	bool attackedByWhiteVerticallyFromDown(int index);


	// ------------------------------------------

	std::string_view blackPieces{ "kqrbnp" };
	std::string_view whitePieces{ "KQRBNP" };

	std::array<double, 6> pieceValues{ 10000.0, 9.0,5.0,3.0, 2.8, 1.0 };

	std::string_view letters{ "abcdefgh" };
	std::string_view numbers{ "12345678" };

	unsigned long long int perftPositions{ 0 };

	std::map<char, double> map;

	std::vector<std::pair<int, int>> av_moves{};

	std::vector<std::pair<ChessMove, BoardState>> stackOfPlayedMoves;
	// start index and end index for a move, a sort of light way to store a move
};

