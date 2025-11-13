"""
Game State Manager - Handles game flow and state
"""

import random
from constants import *
from hex_map import HexMap
from unit import Unit

class Player:
    """Represents a player in the game"""
    
    def __init__(self, side, player_type=PlayerType.HUMAN):
        self.side = side
        self.player_type = player_type
        self.prestige = 500  # Currency for buying units
        self.units = []
        self.objectives_controlled = 0
    
    def get_name(self):
        return SIDE_NAMES[self.side]
    
    def get_color(self):
        return SIDE_COLORS[self.side]
    
    def add_unit(self, unit):
        self.units.append(unit)
    
    def remove_unit(self, unit):
        if unit in self.units:
            self.units.remove(unit)
    
    def get_active_units(self):
        """Get all non-destroyed units"""
        return [u for u in self.units if not u.is_destroyed()]
    
    def end_turn(self):
        """End turn for this player"""
        for unit in self.get_active_units():
            unit.end_turn()


class GameState:
    """Manages overall game state"""
    
    def __init__(self):
        self.hex_map = HexMap()
        self.turn = 1
        self.max_turns = 20
        
        # Create players
        self.players = [
            Player(Side.AXIS, PlayerType.HUMAN),
            Player(Side.ALLIES, PlayerType.AI)
        ]
        self.current_player_idx = 0
        self.current_player = self.players[0]
        
        # Setup initial units
        self.setup_initial_units()
        
        self.game_over = False
        self.winner = None
    
    def setup_initial_units(self):
        """Place initial units on the map"""
        # Axis units (left side)
        axis_positions = [
            (2, 1, "German_Infantry"),
            (3, 2, "German_Tank"),
            (4, 1, "German_Infantry"),
            (5, 2, "German_Artillery"),
            (6, 1, "German_Recon"),
        ]
        
        for row, col, equipment in axis_positions:
            unit = Unit(equipment, Side.AXIS)
            hex_tile = self.hex_map.get_hex(row, col)
            if hex_tile:
                unit.hex = hex_tile
                hex_tile.unit = unit
                self.players[0].add_unit(unit)
        
        # Allied units (right side)
        allies_positions = [
            (2, 14, "Allied_Infantry"),
            (3, 13, "Allied_Tank"),
            (4, 14, "Allied_Infantry"),
            (5, 13, "Allied_Artillery"),
            (6, 14, "Allied_Recon"),
        ]
        
        for row, col, equipment in allies_positions:
            unit = Unit(equipment, Side.ALLIES)
            hex_tile = self.hex_map.get_hex(row, col)
            if hex_tile:
                unit.hex = hex_tile
                hex_tile.unit = unit
                self.players[1].add_unit(unit)
    
    def get_current_player(self):
        """Get the currently active player"""
        return self.current_player
    
    def get_other_player(self):
        """Get the other player"""
        return self.players[1 - self.current_player_idx]
    
    def end_turn(self):
        """End current player's turn"""
        self.current_player.end_turn()
        
        # Switch to next player
        self.current_player_idx = (self.current_player_idx + 1) % len(self.players)
        self.current_player = self.players[self.current_player_idx]
        
        # If back to first player, increment turn
        if self.current_player_idx == 0:
            self.turn += 1
            
        # Check for game over conditions
        self.check_game_over()
    
    def check_game_over(self):
        """Check if game is over"""
        # Check turn limit
        if self.turn > self.max_turns:
            self.game_over = True
            # Winner is player with most objectives
            axis_objs = sum(1 for h_row in self.hex_map.hexes for h in h_row 
                           if h.is_objective and h.owner == Side.AXIS)
            allies_objs = sum(1 for h_row in self.hex_map.hexes for h in h_row 
                             if h.is_objective and h.owner == Side.ALLIES)
            self.winner = Side.AXIS if axis_objs > allies_objs else Side.ALLIES
            return
        
        # Check if any player lost all units
        for player in self.players:
            if len(player.get_active_units()) == 0:
                self.game_over = True
                self.winner = self.get_other_player().side
                return
    
    def get_unit_at(self, hex_tile):
        """Get unit at given hex"""
        return hex_tile.unit if hex_tile else None
    
    def can_unit_move_to(self, unit, target_hex):
        """Check if unit can move to target hex"""
        if not unit or not target_hex:
            return False
        
        if not unit.can_move():
            return False
        
        if not target_hex.is_passable():
            return False

        # Check if hex is occupied by any unit (can't move onto other units)
        if target_hex.unit:
            return False
        
        # Check if within movement range
        reachable = self.hex_map.get_reachable_hexes(unit.hex, unit.get_moves_left())
        return target_hex in reachable
    
    def can_unit_attack(self, unit, target_hex):
        """Check if unit can attack target hex"""
        if not unit or not target_hex or not target_hex.unit:
            return False
        
        if not unit.can_attack():
            return False
        
        # Can't attack own units
        if target_hex.unit.owner == unit.owner:
            return False
        
        # Check if in range (adjacent for most units)
        distance = self.hex_map.distance(unit.hex, target_hex)
        max_range = 2 if unit.get_class() == UnitClass.ARTILLERY else 1
        
        return distance <= max_range
    
    def move_unit(self, unit, target_hex):
        """Move unit to target hex"""
        if not self.can_unit_move_to(unit, target_hex):
            return False
        
        # Calculate path and cost
        path = self.hex_map.find_path(unit.hex, target_hex, unit.get_moves_left())
        if not path:
            return False
        
        # Calculate movement cost
        cost = 0
        for hex_tile in path[1:]:  # Skip starting hex
            cost += hex_tile.get_movement_cost()
        
        # Move unit
        unit.move_to(target_hex, cost)
        
        # Capture objectives
        if target_hex.is_objective:
            target_hex.owner = unit.owner
        
        return True
    
    def attack_unit(self, attacker, target_hex):
        """Execute attack"""
        if not self.can_unit_attack(attacker, target_hex):
            return False
        
        defender = target_hex.unit
        attacker_losses, defender_losses = attacker.attack(defender)
        
        # Remove destroyed units
        if defender.is_destroyed():
            target_hex.unit = None
            self.get_other_player().remove_unit(defender)
        
        if attacker.is_destroyed():
            attacker.hex.unit = None
            self.current_player.remove_unit(attacker)
        
        return True
