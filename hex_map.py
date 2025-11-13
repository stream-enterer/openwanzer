"""
Hex Map System - Based on axial coordinate system
"""

import math
import random
from constants import *

class Hex:
    """Represents a single hex tile on the map"""
    
    def __init__(self, row, col, terrain=TerrainType.CLEAR):
        self.row = row
        self.col = col
        self.terrain = terrain
        self.unit = None
        self.is_objective = False
        self.owner = None  # Side that controls this hex (for cities/objectives)
        
    def get_pixel_position(self, offset_x=100, offset_y=50):
        """Convert hex coordinates to screen pixel position"""
        # Axial to pixel conversion for flat-top hexes
        x = offset_x + HEX_SIZE * 1.5 * self.col
        # Offset every other column
        y = offset_y + HEX_HEIGHT * (self.row + 0.5 * (self.col % 2))
        return (x, y)
    
    def get_movement_cost(self):
        """Get the movement cost for this terrain"""
        return MOVEMENT_COSTS.get(self.terrain, 1)
    
    def get_defense_bonus(self):
        """Get the defense bonus for this terrain"""
        return TERRAIN_DEFENSE.get(self.terrain, 0)
    
    def is_passable(self):
        """Check if this hex is passable"""
        return self.terrain != TerrainType.WATER
    
    def __repr__(self):
        return f"Hex({self.row}, {self.col}, {TERRAIN_NAMES[self.terrain]})"


class HexMap:
    """Manages the hex-based game map"""
    
    def __init__(self, rows=MAP_ROWS, cols=MAP_COLS):
        self.rows = rows
        self.cols = cols
        self.hexes = []
        self.generate_map()
        
    def generate_map(self):
        """Generate a random tactical map"""
        self.hexes = []
        
        for row in range(self.rows):
            hex_row = []
            for col in range(self.cols):
                # Generate terrain with weighted probabilities
                rand = random.random()
                if rand < 0.5:
                    terrain = TerrainType.CLEAR
                elif rand < 0.65:
                    terrain = TerrainType.FOREST
                elif rand < 0.75:
                    terrain = TerrainType.MOUNTAIN
                elif rand < 0.8:
                    terrain = TerrainType.CITY
                else:
                    terrain = TerrainType.CLEAR
                    
                hex_tile = Hex(row, col, terrain)
                hex_row.append(hex_tile)
                
            self.hexes.append(hex_row)
        
        # Place some objectives (cities)
        self.place_objectives()
        
        # Add some roads
        self.add_roads()
    
    def place_objectives(self):
        """Place objective cities on the map"""
        objectives = [
            (self.rows // 3, self.cols // 4),
            (self.rows // 3, 3 * self.cols // 4),
            (2 * self.rows // 3, self.cols // 2)
        ]
        
        for row, col in objectives:
            if 0 <= row < self.rows and 0 <= col < self.cols:
                self.hexes[row][col].terrain = TerrainType.CITY
                self.hexes[row][col].is_objective = True
    
    def add_roads(self):
        """Add a simple road network"""
        # Horizontal road
        road_row = self.rows // 2
        for col in range(self.cols):
            if self.hexes[road_row][col].terrain == TerrainType.CLEAR:
                self.hexes[road_row][col].terrain = TerrainType.ROAD
    
    def get_hex(self, row, col):
        """Get hex at given position"""
        if 0 <= row < self.rows and 0 <= col < self.cols:
            return self.hexes[row][col]
        return None
    
    def get_neighbors(self, hex_tile):
        """Get all valid neighboring hexes (6 directions for axial coordinates)"""
        row, col = hex_tile.row, hex_tile.col
        
        # Axial coordinate neighbors (flat-top orientation)
        if col % 2 == 0:  # Even column
            neighbors = [
                (row - 1, col),      # North
                (row, col + 1),      # Northeast
                (row + 1, col + 1),  # Southeast
                (row + 1, col),      # South
                (row + 1, col - 1),  # Southwest
                (row, col - 1)       # Northwest
            ]
        else:  # Odd column
            neighbors = [
                (row - 1, col),      # North
                (row - 1, col + 1),  # Northeast
                (row, col + 1),      # Southeast
                (row + 1, col),      # South
                (row, col - 1),      # Southwest
                (row - 1, col - 1)   # Northwest
            ]
        
        valid_neighbors = []
        for r, c in neighbors:
            hex_n = self.get_hex(r, c)
            if hex_n:
                valid_neighbors.append(hex_n)
        
        return valid_neighbors
    
    def get_reachable_hexes(self, start_hex, movement_points):
        """
        Get all hexes reachable from start_hex with given movement points.
        Returns dict of {hex: cost}
        """
        if not start_hex:
            return {}
        
        frontier = [(start_hex, 0)]
        visited = {start_hex: 0}
        
        while frontier:
            current_hex, cost = frontier.pop(0)
            
            for neighbor in self.get_neighbors(current_hex):
                move_cost = neighbor.get_movement_cost()
                new_cost = cost + move_cost

                # Check if we can reach this hex (must be passable and not occupied by a unit)
                if new_cost <= movement_points and neighbor.is_passable() and not neighbor.unit:
                    # If not visited or found a cheaper path
                    if neighbor not in visited or new_cost < visited[neighbor]:
                        visited[neighbor] = new_cost
                        frontier.append((neighbor, new_cost))
        
        return visited
    
    def find_path(self, start_hex, end_hex, movement_points):
        """
        Find path from start to end using A* algorithm.
        Returns list of hexes or None if no path found.
        """
        if not start_hex or not end_hex or not end_hex.is_passable():
            return None
        
        def heuristic(h1, h2):
            """Manhattan distance for hex grid"""
            return abs(h1.row - h2.row) + abs(h1.col - h2.col)
        
        frontier = [(0, start_hex, [start_hex], 0)]  # (priority, hex, path, cost)
        visited = {start_hex: 0}
        
        while frontier:
            frontier.sort(key=lambda x: x[0])
            _, current, path, cost = frontier.pop(0)
            
            if current == end_hex:
                return path
            
            for neighbor in self.get_neighbors(current):
                move_cost = neighbor.get_movement_cost()
                new_cost = cost + move_cost

                # Path through passable hexes without units
                if new_cost <= movement_points and neighbor.is_passable() and not neighbor.unit:
                    if neighbor not in visited or new_cost < visited[neighbor]:
                        visited[neighbor] = new_cost
                        priority = new_cost + heuristic(neighbor, end_hex)
                        new_path = path + [neighbor]
                        frontier.append((priority, neighbor, new_path, new_cost))
        
        return None
    
    def distance(self, hex1, hex2):
        """Calculate distance between two hexes"""
        return abs(hex1.row - hex2.row) + abs(hex1.col - hex2.col)
