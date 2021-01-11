#include "../include/HexVector.h"
#include <iostream>
#include <random>

using namespace std;
using namespace HexOguz;

HexVector::HexVector(): AbstractHex(0)
{
    createTable();
}

HexVector::HexVector(int size): AbstractHex(size)
{
    createTable();
}

void HexVector::print() const
{
    // these things just showing a beautiful table. 
    cout << "  ";
    for (int i = 0; i < size; i++) cout << static_cast<char> ('a' + i)  << " ";
    cout << endl; 
    for (int i = 0; i < size; i++)
    {
        if (i < 9)
            cout << i+1 << " ";
        else
            cout << i+1;
        for (int k = 0; k < i; k++) cout << " ";
        for (int j = 0; j < size; j++)
        {
            cout << " " << hexCells[i][j].getState();
        }
        cout << endl;
    }
}

void HexVector::reset()
{
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            hexCells[i][j].setState(dot);
}

void HexVector::createTable()
{
    for (int i = 0; i < size; i++)
    {
        hexCells.push_back(vector<Cell>());
        keepUppercase.push_back(vector<Cell>());
        for (int j = 0; j < size; j++)
        {
            hexCells[i].push_back(Cell());
            keepUppercase[i].push_back(Cell());
        }
    }
}

void HexVector::setSize(int size)
{
    this->size = size;
    createTable();
    reset();
}

void HexVector::readFromFile(string filename)
{
    // rading mode
	ifstream fp(filename);
	if(fp.is_open() == false)
    {
		cerr << "Couldn't open file.\n";  return;
	}

    // keep old gameType 
    int tempType = gameType;

    // read variables from file
	fp >> gameType >> size  >> turn;

    // resize table
    hexCells.resize(size);
    keepUppercase.resize(size);
    for (int i = 0; i < size; i++)
    {   
        keepUppercase[i].resize(size);
        hexCells[i].resize(size);
    }
 
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            char temp;
            fp >> temp;
            hexCells[i][j].setState(temp);
        }
    }
	
    fp.close();

    
    if (gameType == 2)
        turn = player1;
    cout << "Game Loaded from -> " + filename << endl;

    if (tempType != gameType)
            cout << "Game Type is changed" << endl;
        cout << "There is new table" << endl;

}

void HexVector::writeToFile(string filename)
{
    // writing mode
	ofstream fp(filename);
	if(fp.is_open() == false)
    {
		cerr << "Could not open the file\n";  return;
	}

    // write those to file
	fp << gameType << endl << size << endl << turn << endl;

    // write table as type of char
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++)
            fp << hexCells[i][j].getState();
        fp << endl;
    }

	cout << "Game saved as -> " + filename << endl;
	fp.close();  return;
}

Cell HexVector::operator()(int column, int row) const
{
    return hexCells[column][row];
}

void HexVector::play()
{
    totalMove++;
    // CPU plays random
    setTurn(player2);
    random_device randomCreator;
    mt19937 rand(randomCreator());
    uniform_int_distribution<mt19937::result_type> randRange(0, size -1);
    int row = randRange(randomCreator);
    int column = randRange(randomCreator);
    while (hexCells[column][row].getState() != dot)
    {
        row = randRange(randomCreator);
        column = randRange(randomCreator);
    }
    char row_c = row + 'A';
    cout << "Your Turn " << turn << endl;
    cout << "Computer Entered: " << row_c << column + 1 << endl;
    hexCells[column][row].setState(turn);
    moveLast = Cell(row,column,turn);
    print();
}

void HexVector::play(Cell position)
{
    totalMove++;
    turn = position.getState();
    hexCells[position.getColumn()][position.getRow()].setState(position.getState());
    moveLast = position;
    print();
}

void HexVector::playGame()
{
    Welcome();
    createTable();

    string coordinate;
    Cell tempCell;
    while(true)
    {

        tempCell.setState(player1);
        while(checkCommand(coordinate, tempCell)); 
        // If gamestate turns to passive, then retrun to main
        if (gameState == passive) return;
        play(tempCell);
        // if there is a winner, break
        if (checkWinner()) break;

        // if gametype is PvCPU
        if (gameType == PvCPU)
            play();
        else
        {
            tempCell.setState(player2);
            while(checkCommand(coordinate, tempCell));
            if (gameState == passive) return;
            // if gameType has changed
            if (gameType == PvCPU){ print(); continue; }
            play(tempCell);
        }
        if (checkWinner()) break;
    }
    
    // it means there is a winner
    if (gameState == winner)
        copyToUpper();
}

bool HexVector::checkCommand(string coordinate, Cell& tempCell)
{
    cout << endl << "Your Turn " << tempCell.getState() << endl;
    cout << "Type H for all Commands" << endl << "Command: ";
    cin >> ws;
    getline(cin, coordinate);

    if (coordinate[0] == 'H' && coordinate.length() == 1)
    {
        allCommands();
        return checkCommand(coordinate, tempCell);
    }
    else if (coordinate[0] == 'Q' && coordinate.length() == 1)
    {
        cout << "Quiting... " << endl;

        gameState = passive;
        cout << "This game will be active in background" << endl;
        return false;

    }
    else if (coordinate == "LAST" && coordinate.length() == 4)
    {
        cout << "Last move ->"; 
        lastMove().print();
        return checkCommand(coordinate, tempCell);
    }
    else if (coordinate[0] == 'P' && coordinate.length() == 1)
    {
        print();
        return checkCommand(coordinate, tempCell);
    }
    if (saveOrLoad(coordinate))
        return true;

    // in load circumstance, this func should return false.
    else if(coordinate.compare(0, 4, "LOAD", 0 ,4) == 0 && coordinate.size() > 5) 
        return false;
    else
    {
        int row = coordinate[0] - 'A';
        int column = coordinate[1] - '1';
        if (coordinate.length() == 3)
            column = (coordinate[1] - '0') * 10 + (coordinate[2] - '1');
        if ((coordinate.length() != 2 && coordinate.length() != 3) || row > size - 1 || row < 0 || column < 0 || column > size - 1)
        {
            cerr << "Your input is not correct. Be aware this game is case sensitive" << endl;
            return checkCommand(coordinate, tempCell);
        }
        if (hexCells[column][row].getState() != dot)
        {   
            cerr << "this filed is not empty" << endl;
            return checkCommand(coordinate, tempCell);
        }
        tempCell.setRow(row);
        tempCell.setColumn(column);
        return false;
    }
}

bool HexVector::checkWinner()
{
    int x = 0, y = 0;
    // if player is x then every time look for
    // up to down
    // this means start looking for x values from top
    // to bottom till found an x value 
    if (turn == player1)
    {
        for (int i = 0; i < size; i++)
        {
            if(check(x + i, y))
            {
                gameState = winner;
                return true;
            }
        }
    }
    // if player is o then everytime look
    // right to left
    // this means start looking for o values from left side and then
    // search till end of the size to right
    else if (turn == player2)
    {
        for (int i = 0; i < size; i++)
        {
            if(check(x, y + i))
            {
                gameState = winner;
                return true;
            }
        }
    }
    return false;
}

bool HexVector::check(int x, int y)
{
    if (turn == player1)
    {
        if (y == size - 1 && x >= 0 && hexCells[x][y].getState() == turn) {
            keepUppercase[x][y].setState(turn - 32); 
            return true; 
        }
    }
    else if (turn == player2)
    {
        if (x == size - 1 && y >= 0 && hexCells[x][y].getState() == turn) {
            keepUppercase[x][y].setState(turn - 32); 
            return true; 
        }
    }
  
    if (isValid(x, y)) { 
        keepUppercase[x][y].setState(turn - 32); 
  
        // move down
        if (check(x + 1, y)) 
            return true; 
  
        // move right
        if (check(x, y + 1)) 
            return true; 

        // if player is x then move up right corner
        if (turn == player1)
        {
            if (check(x - 1, y + 1)) 
                return true; 
        }
        // if player is o then move down left corner
        else if (turn == player2)
        {
            if (check(x + 1, y - 1)) 
                return true; 
        }

        // if none of them can return true then clear data from keepPoints array
        keepUppercase[x][y].setState(dot); 
        return false; 
    } 
  
    return false; 
}

bool HexVector::isValid(int x, int y)
{
    // if this field is valid return true
    if (x >= 0 && x < size && y >= 0 && y < size && hexCells[x][y].getState() == turn)
        return true; 

    return false; 
}

void HexVector::copyToUpper()
{
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            if (keepUppercase[i][j].getState() != dot)
                hexCells[i][j] = keepUppercase[i][j];

    cout << endl << "You win  ->>>>> " << static_cast<char>(turn - 32) << " <<<<<<<-  " << endl<< "Congrats You are the Pro Hex Player <3<3" << endl << endl;
    print();
    gameState = winner;
    if (isEnd())
        cout << "This game is passive now" << endl;
    activeGames--;
    cout << "There are " << activeGames << " active games now" << endl; 

}