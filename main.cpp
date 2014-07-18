#include <iostream>
#include <iomanip>
#include <stdint.h>
#include <random>
#include <assert.h>

typedef unsigned char TFIELDVAL;
typedef uint64_t UBIGINT;

class RandomWrapper{
private:
	std::mt19937  generator;
	bool bFirstCall = true;
	UBIGINT nSeedVal;
	UBIGINT nDistLength;
	std::uniform_int_distribution<int> distribution;
	std::discrete_distribution<> disc_dist;
public:
	RandomWrapper(){}
	RandomWrapper(double dWeight){
		nSeedVal = 536452652462461753;
		nDistLength = 1000;
		
		generator.seed(nSeedVal);
		distribution = std::uniform_int_distribution<int> (0,nDistLength-1);
		disc_dist = std::discrete_distribution<> ({dWeight, 1-dWeight});
	}
	int getRand(int nMin, int nMax) {    
		double x = distribution(generator)*1.0/((nDistLength*1.0)/((nMax+1-nMin)*1.0));
		int n = floor(x) + nMin;
		return (n);
	}
	int getDiscDist(int nVal1,int nVal2){
		return ((disc_dist(generator) == 0) ? nVal1 : nVal2);
	}
};
class G2048Board {
private:	
    const TFIELDVAL nInitVal = 0; //
    const int nSize = 4;
    const int nArea = nSize*nSize;
    const int nMaxVal = 11; // 2048
    const double dWeight2 = 0.9;
	const double dWeight4 = 1-dWeight2;
    TFIELDVAL* aFields;
    // some properties
    int cntZero;
    bool bChangeable;
	bool mBTouched = false;
	TFIELDVAL mNCurMaxVal;
	RandomWrapper rw;
public:
    G2048Board() {
        init();
    }
    G2048Board(const unsigned int aVal[]){
		init();
		unsigned int nVal;
		TFIELDVAL nLogVal;
		for(int j = 0;j < nArea;j++){
			nLogVal = -1;
			nVal = aVal[j];
			if (nVal == 0){
				setFVal(j, 0);
				continue;
			}
			while (nVal != 0){
				nVal >>= 1;
				nLogVal++;
			}
			setFVal(j, nLogVal);
		}		
    }
    G2048Board(const G2048Board& gbSrc) {
        aFields = new TFIELDVAL[nArea];
        copyFrom(gbSrc);
        update();
    }
    G2048Board& operator= (const G2048Board &gbSource)
    {
        aFields = new TFIELDVAL[nArea];
        copyFrom(gbSource);
        return *this;
    }
    void copyFrom(const G2048Board &gbSrc) {
        for (int j=0; j<nSize; j++) {
            for (int k=0; k<nSize; k++) {
                setFVal(j,k,gbSrc.getFVal(j,k));
            }
        }
        update();
    }
    inline bool IsChangeable() const {
		return (bChangeable);
    }
    bool IsWon(){
		return(mNCurMaxVal >= nMaxVal);
    }
    // note: this looks strange, but we might change the eval function !
    inline int eval() const{
		return (cntZero);
    }
    inline int getCntZeroes() const{
		return (cntZero);
    }
private:	
    inline void reset(){
		
    }
    // access like vector
    inline TFIELDVAL getFVal(int nPos) const {
        return (aFields[nPos]);
    }
    // access like matrix
    inline TFIELDVAL getFVal(int nRow, int nCol) const {
        return (getFVal(nRow * nSize + nCol));
    }
    inline TFIELDVAL getFVal(int nRow, int nCol, bool bCorrectOrder) const {
        if (bCorrectOrder) {
            return getFVal(nRow, nCol);
        } else {
            return getFVal(nCol, nRow);
        }
    }
	inline void setFVal(int nPos, TFIELDVAL nVal) {
		if (getFVal(nPos) != nVal){
			mBTouched = true;
// 			std::cout << "setFVal(): nPos= " << nPos << ", nVal = " << (int)nVal << std::endl;
			aFields[nPos] = nVal;
		}
	}	
    inline void setFVal(int nRow, int nCol, TFIELDVAL nVal) {
		setFVal(nRow * nSize + nCol, nVal);
    }
    inline void setFVal(int nRow, int nCol, TFIELDVAL nVal, bool bCorrectOrder) {
        if (bCorrectOrder) {
            setFVal(nRow, nCol, nVal);
        } else {
            setFVal(nCol, nRow, nVal);
        }
    }
    inline bool IsDirty(){
		bool bTouched = mBTouched;
		mBTouched = false;
		return (bTouched);
    }
public:    
    void init(bool bStartBoard = true) {
		rw = RandomWrapper(dWeight2);
        aFields = new TFIELDVAL[nArea];
		
        for (int j=0; j<nSize; j++) {
            for (int k=0; k<nSize; k++) {
                setFVal(j,k,nInitVal);
            }
        }
        if (bStartBoard) {
            int nField1 = rw.getRand(0,nArea -1);
            int nField2;
            while ((nField2 = rw.getRand(0,nArea -1)) == nField1);
            aFields[nField1] = rw.getDiscDist(1,2);
            aFields[nField2] = rw.getDiscDist(1,2);
        }
        update();
    }
private:    
    void update() {
        // cnt nr of zeroes and maximum value
        cntZero = 0;
		mNCurMaxVal = 0;
        for (int j=0; j<nArea; j++) {
            if(aFields[j] == 0) {
                cntZero++;
            } else {
				mNCurMaxVal = (mNCurMaxVal < aFields[j]) ? aFields[j] : mNCurMaxVal;
            }
        }
        bChangeable == (cntZero > 0);
        if (!bChangeable) { // test row neighbours
            for(int j=0; j<nSize && !bChangeable; j++) {
                for(int k=0; k<nSize-1; k++) {
                    if (getFVal(j,k) == getFVal(j,k+1)) {
                        bChangeable = true;
                    }
                }
            }
        }
        if (!bChangeable) {
            for(int k=0; k<nSize && !bChangeable; k++) {
                for(int j=0; j<nSize-1; j++) {
                    if (getFVal(j,k) == getFVal(j+1,k)) {
                        bChangeable = true;
                    }
                }
            }
        }
    }
public:
    // nDir: +1 right, -1 left, +2 down, -2: up
	// return true <=> board has changed
    bool move(int nDir) {
        TFIELDVAL nVal;
        int j,k,l;
        assert(nSize==4);
        bool bIsHorizontal = (nDir % 2 != 0);
        nDir = (nDir > 0) ? 1 : -1;
        int nFirst = (nDir == 1) ? nSize - 1: 0;
        int nLast  = nSize - nFirst - 1;
		int nLTest;
		bool bDirty = IsDirty();
        for (j=0; j<4; j++) {
            for(k=l=nFirst; nDir * (k - nLast) >= 0; k-=nDir,l-=nDir) {
                nVal = 0;
                while (nDir * (l - nLast) >= 0 && (nVal = getFVal(j,l,bIsHorizontal)) == 0) {
                    l-=nDir;
                }
                setFVal(j,k,nVal,bIsHorizontal);

            }

            for(k=l=nFirst; nDir * (k - nLast) >= 0; k-=nDir,l-=nDir) {
                nVal = getFVal(j,l,bIsHorizontal);
                if ((nLTest = nDir * (l - nLast)) >= 0) {
                    if(nVal != 0 && nLTest > 0 && nVal == getFVal(j,l-nDir,bIsHorizontal)) {
                        setFVal(j,k,nVal+1,bIsHorizontal);
                        l-=nDir;
                    } else {
                        setFVal(j,k,nVal,bIsHorizontal);
                    }
                } else {
                    setFVal(j,k,0,bIsHorizontal);
                }
            }
        }
        bDirty = IsDirty();
		if (bDirty) {
			update();
		}
		return (bDirty);
    }
private:    
	bool addRandomTile(){
		return(addTile(rw.getRand(1,cntZero), rw.getDiscDist(1,2)));
	}
public:
	// for bot: to generate all possible random tiles that can be added
	bool addTile(int nZeroPos, TFIELDVAL nVal){
		if (cntZero == 0){
			return (false);
		}
		for(int j=0, nZCnt=0;j<nArea;j++){
			if (aFields[j] == 0){
				nZCnt++;
				if (nZCnt == nZeroPos){
					aFields[j] = nVal;
					update(); // alt: cntZero--;
					return(true);
				}
			}
		}
		return(false);
	}	
	bool moveWTile (int nMoveDir){
		move(nMoveDir);
		addRandomTile();
	}
    bool moveRight(bool bAddTile = true) {
        move(1);
		if (bAddTile){
			addRandomTile();
		}
    }
    bool moveLeft(bool bAddTile = true) {
        move(-1);
		if (bAddTile){
			addRandomTile();
		}		
    }

    bool moveDown(bool bAddTile = true) {
        move(2);
		if (bAddTile){
			addRandomTile();
		}		
    }
    bool moveUp(bool bAddTile = true) {
        move(-2);
		if (bAddTile){
			addRandomTile();
		}		
    }

    G2048Board cpMove (int nMove) const{
        G2048Board g = *this;
// 		g.copyFrom(*this);
        g.move(nMove);
        return (g);
    }
    
    G2048Board cpMoveRight(bool bAddTile = true) {
        G2048Board g = *this;
// 		g.copyFrom(*this);
        g.moveRight(bAddTile);
        return (g);
    }
    G2048Board cpMoveLeft(bool bAddTile = true) {
        G2048Board g = *this;
// 		g.copyFrom(*this);
        g.moveLeft(bAddTile);
        return (g);
    }
    G2048Board cpMoveDown(bool bAddTile = true) {
        G2048Board g = *this;
// 		g.copyFrom(*this);
        g.moveDown(bAddTile);
        return (g);
    }
    G2048Board cpMoveUp(bool bAddTile = true) {
        G2048Board g = *this;
// 		g.copyFrom(*this);
        g.moveUp(bAddTile);
        return (g);
    }

    UBIGINT explicitVal(int nRow, int nCol) {
        TFIELDVAL nVal = getFVal(nRow, nCol);
        return (nVal == nInitVal) ? 0 : 1 << nVal;
    }

    std::string pretty_print() {
        const char cLeftCorner = 169;
        const char cRightCorner = 170;
        std::string s = "";
// 		s = s.append(&cLeftCorner).append(&cRightCorner);
        std::cout << std::setw(4);
        for (int j=0; j<nSize; j++) {
            for (int k=0; k<nSize; k++) {
                std::cout << std::setw(5) << explicitVal(j,k);
            }
            std::cout << std::endl;
        }
        return (s);
    }
};
class Bot {
private:
	G2048Board g;
	bool mBDebug = false;
	const int mNLookAhead = 3;
	int mNCurDepth = 0;
	int mNBestMove = 0;
	const std::string sMoves[5]  = {"up","left","dummy", "right", "down"};
public:
	void setBoard(const G2048Board& gb){
		g = gb;
	}
	int getBestMove(){
		search();
		return (mNBestMove);
	}
	void search(){
// 		G2048Board* gbBuffer = new G2048Board (2*mNLookAhead);
		mNCurDepth = 0;
		evaluateLeaf(g);
	}

	inline std::string movStr(int j){
		return (sMoves[j+2]);
	}
private:	
	double evaluateLeaf(const G2048Board& gb){		
		mNCurDepth++;
// 		debugprint(std::string("evaluateLeaf: start: mNCurDepth: ") + std::to_string(mNCurDepth));		
		if (mNCurDepth == mNLookAhead || !gb.IsChangeable()){
			mNCurDepth--;
			return(gb.eval());
		}
		G2048Board gbMoved, gbAddedTile;
		int cntZero;
		int cnt = 0;
		double dEvalSum, dEvalSum4Move, dMaxVal = 0.0;
		int nBestMove;
		for (int nMove = -2;nMove <= 2;nMove++){
			if (nMove == 0){
				continue;
			}
			dEvalSum4Move = 0.0;
			gbMoved = gb;
			if (!gbMoved.move(nMove)){
				continue;
			}
// 			debugprint(gbMoved.pretty_print());
			cntZero = gbMoved.getCntZeroes();
			cnt += 2*cntZero;
			for(int nZero=0;nZero < cntZero;nZero++){
				gbAddedTile = gbMoved;
				gbAddedTile.addTile(nZero, 1);
				dEvalSum4Move += evaluateLeaf(gbAddedTile);
				gbAddedTile = gbMoved;
				gbAddedTile.addTile(nZero, 2);
				dEvalSum4Move += evaluateLeaf(gbAddedTile);				
			}
			if (dEvalSum4Move /(2*cntZero) > dMaxVal){
				nBestMove = nMove;
				dMaxVal = dEvalSum4Move /(2*cntZero);
			}
			dEvalSum += dEvalSum4Move;
			debugprint(std::string("Depth: ") + std::to_string(mNCurDepth));
			debugprint(std::string("Move: ") + movStr(nMove));
			debugprint(std::string("Sum : ") + std::to_string(dEvalSum4Move));
		}
		if (mNCurDepth == 1){
			mNBestMove = nBestMove;
			debugprint(std::string("Best Move: ") + movStr(nBestMove));
		}
		mNCurDepth--;
		return (dEvalSum/cnt);
	}
	void debugprint(std::string s) const{
		if (mBDebug){
			std::cout << s << std::endl;
		}
	}
};

int main(int argc, char **argv) {
// 	for(int j=0;j<30;j++){
// 		std::cout << getRand(0,10) << " ";
// 	}
// 	std::cout << std::endl;
	unsigned int err1 [16] = {8, 128, 32, 8, 16, 256, 16, 2, 2, 4, 0, 0, 0, 0, 0, 0};
// 	unsigned int err1 [16] = {8, 128, 32, 8,  2,  16, 256, 16, 2, 4, 0, 0, 0, 0, 0, 0};
// 	unsigned int err1 [16] = {8, 128, 32, 8, 16, 256, 16, 4, 4, 0, 0, 0, 0, 0, 0, 0};
	
// 	G2048Board gb1(err1);	
// 	std::cout << gb1.pretty_print() << std::endl;
// 	gb1.moveLeft(false);
// // 	gb1.moveRight();
// 	std::cout << gb1.pretty_print() << std::endl;
	
    G2048Board gb1, gb2;
	Bot oBot;
	int nMove;
	gb1.init();
	for (int j=0;j<1000;j++){
		std::cout << "gb1:" << j << std::endl;
		std::cout << gb1.pretty_print() << std::endl;
		oBot.setBoard(gb1);
		nMove = oBot.getBestMove();
		gb1.moveWTile(nMove);
		std::cout << oBot.movStr(oBot.getBestMove()) << std::endl;
		if (gb1.IsWon()){
			std::cout << "You win !"  << std::endl;
			break;
		}
		if (!gb1.IsChangeable()){
			std::cout << "You have lost !"  << std::endl;
			break;			
		}
	}
//     for(int j=0; j<10; j++) {
//         gb1.init();
//         std::cout << "init gb1:" << std::endl;
//         std::cout << gb1.pretty_print() << std::endl;
//         gb2 = gb1.cpMoveUp();
//         std::cout << "move up: gb2 :" << std::endl;
//         std::cout << gb2.pretty_print() << std::endl;
//         std::cout << "init gb1:" << std::endl;
//         std::cout << gb1.pretty_print() << std::endl;
// 
//     }
    return 0;
}
