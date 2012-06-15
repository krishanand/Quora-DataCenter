/*
   A fast, elegant solution to the Quora DataCenter Challenge.
   Author: Anand Krishnamoorthi
*/

#if defined(__GCC__)
 #define FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
 #define FORCE_INLINE __forceinline
 #define _SECURE_SCL 0
#else
 #define FORCE_INLINE inline
#endif

#include <vector>
#include <iostream>
#include <cassert>

struct Room {
	Room* path_end;     
	int   connections;
};

// A connection between a room and it's neighbor (either right or down)
struct Connection {
	Room* room;
	Room* neighbor;

	Room* room_end;
	Room* neighbor_end;
	bool  connected;	

	FORCE_INLINE Connection(Room* r, Room* n)
	{
		room = r;
		neighbor = n;
		connected = false;

		// Make sure rooms are not fully connected..
		if (room->connections != 2 && neighbor->connections != 2) { 
			// Check for cycles. This is a key semantic and performance constraint.
			if (room != neighbor->path_end) {
				room_end       = room->path_end;
				neighbor_end   = neighbor->path_end; 

				/* sanity checks*/
				assert(room_end->path_end == room); 
				assert(neighbor_end->path_end == neighbor);
				connected = true;

				// Update ends of the merged path to point to each other
				room_end->path_end     = neighbor_end;   
				neighbor_end->path_end = room_end;

				room->connections++;
				neighbor->connections++;				
			}
		}
	}

	FORCE_INLINE ~Connection()
	{
		if (connected) {
			// Restore path end-points.
			room_end->path_end     = room;
			neighbor_end->path_end = neighbor;			

			room->connections--;
			neighbor->connections--;
		}
	}

	FORCE_INLINE operator bool() const { return connected; }
};

void solve(std::vector< std::vector<Room> >& datacenter, int row, int col, int* solutions)
{
	const int R = datacenter.size()    - 1;  // Account for extra row
	const int C = datacenter[0].size() - 1;  // and coloumn

	if (col == C) {							 // Wrap to next row.
		col = 0;
		if(++row == R) {                     // Final row means that all rooms have
			++*solutions;					 // been connected. A solution. 
			return;
		}
	}

	Room* room       = &datacenter[row][col];
	Room* room_right = &datacenter[row][col+1];
	Room* room_down  = &datacenter[row+1][col];

	if (room->connections == 2)              
		return solve(datacenter, row, col+1, solutions);	

	if (room->connections == 0) {            
		// Room *must* be connected on both right and down.
		if (const Connection& connect_right = Connection(room, room_right))
			if(const Connection& connect_down = Connection(room, room_down))
				solve(datacenter, row, col+1, solutions);
		return;
	}
	
	// room->connections == 1. 
	// Connect first to the right and explore.
	if (const Connection& connect_right = Connection(room, room_right))
		solve(datacenter, row, col+1, solutions);

	// Then connect down and explore.
	if (const Connection& connect_down = Connection(room, room_down))
		solve(datacenter, row, col+1, solutions);
}

int main(int argc, char** argv)
{
	std::vector< std::vector<Room> > datacenter;
	int solutions = 0;
	int R = 0;
	int C = 0;

	std::cin >> C;
	std::cin >> R;

	datacenter.resize(R+1);
	for(int r=0; r < R+1; ++r) {
		datacenter[r].resize(C+1);
		for(int c=0; c < C+1; ++c) {
			Room* room = &datacenter[r][c];
			room->path_end = room;                    // This assignment simplifies room connection.
			int kind = 1;
			if (r < R && c < C)
				std::cin >> kind;			
			switch (kind) {
			case 0: room->connections = 0; break;     // A room we own. It has not been connected.
			case 1: room->connections = 2; break;     // Unowned room. Assume it is fully connected.
			case 2: room->connections = 1; break;     // Start room. Assume there's a connection from an external source.
			case 3: room->connections = 1; break;     // End room. Assume there's a connection to an external destination.
			default: assert(false); break;
			}			
		}
	}

	solve(datacenter, 0, 0, &solutions);	
	std::cout<<solutions;
}
