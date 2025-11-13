"""
Simple AI for the computer player
"""

import random
from constants import *

class SimpleAI:
    """Basic AI that can play the game"""
    
    def __init__(self, game_state, player):
        self.game_state = game_state
        self.player = player
    
    def get_action(self):
        """
        Decide on next action for AI.
        Returns tuple: (action_type, unit, target_hex)
        """
        # Get all active units
        units = self.player.get_active_units()
        if not units:
            return (ActionType.END_TURN, None, None)
        
        # Shuffle units for variety
        random.shuffle(units)
        
        # Try to attack with any unit
        for unit in units:
            if unit.can_attack():
                target = self.find_attack_target(unit)
                if target:
                    return (ActionType.ATTACK, unit, target)
        
        # Try to move units towards objectives or enemies
        for unit in units:
            if unit.can_move():
                target = self.find_move_target(unit)
                if target:
                    return (ActionType.MOVE, unit, target)
        
        # No more actions, end turn
        return (ActionType.END_TURN, None, None)
    
    def find_attack_target(self, unit):
        """Find a good target to attack"""
        if not unit.hex:
            return None
        
        # Get neighboring hexes
        neighbors = self.game_state.hex_map.get_neighbors(unit.hex)
        
        # Artillery can attack at range 2
        if unit.get_class() == UnitClass.ARTILLERY:
            extended_neighbors = []
            for neighbor in neighbors:
                extended_neighbors.extend(self.game_state.hex_map.get_neighbors(neighbor))
            neighbors = extended_neighbors
        
        # Find enemy units in range
        targets = []
        for hex_tile in neighbors:
            if hex_tile.unit and hex_tile.unit.owner != unit.owner:
                if self.game_state.can_unit_attack(unit, hex_tile):
                    targets.append(hex_tile)
        
        if not targets:
            return None
        
        # Prioritize weak targets
        targets.sort(key=lambda h: h.unit.strength)
        return targets[0]
    
    def find_move_target(self, unit):
        """Find a good hex to move to"""
        if not unit.hex:
            return None
        
        # Get reachable hexes
        reachable = self.game_state.hex_map.get_reachable_hexes(
            unit.hex, unit.get_moves_left()
        )
        
        if not reachable:
            return None
        
        # Filter out occupied hexes
        valid_targets = [h for h in reachable.keys() 
                        if not h.unit and h != unit.hex]
        
        if not valid_targets:
            return None
        
        # Find objectives
        objectives = [h for h in valid_targets if h.is_objective]
        
        # Prioritize uncaptured or enemy objectives
        enemy_objectives = [h for h in objectives 
                           if h.owner != unit.owner]
        if enemy_objectives:
            # Move towards closest enemy objective
            enemy_objectives.sort(
                key=lambda h: self.game_state.hex_map.distance(unit.hex, h)
            )
            return enemy_objectives[0]
        
        # Find nearest enemy unit
        enemy_units = []
        for player in self.game_state.players:
            if player.side != unit.owner:
                enemy_units.extend([u for u in player.get_active_units() if u.hex])
        
        if enemy_units:
            # Move towards nearest enemy
            best_hex = None
            best_distance = float('inf')
            
            for hex_tile in valid_targets:
                min_enemy_dist = min(
                    self.game_state.hex_map.distance(hex_tile, enemy.hex)
                    for enemy in enemy_units
                )
                if min_enemy_dist < best_distance:
                    best_distance = min_enemy_dist
                    best_hex = hex_tile
            
            if best_hex:
                return best_hex
        
        # Default: move towards center or forward
        center_col = self.game_state.hex_map.cols // 2
        valid_targets.sort(
            key=lambda h: abs(h.col - center_col)
        )
        return valid_targets[0] if valid_targets else None
