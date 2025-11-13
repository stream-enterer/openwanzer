"""
Unit class - Represents military units in the game
"""

from constants import *

class Unit:
    """Represents a military unit"""
    
    _id_counter = 0
    
    def __init__(self, equipment_id, owner_side):
        self.id = Unit._id_counter
        Unit._id_counter += 1
        
        self.equipment_id = equipment_id
        self.data = UNIT_DATA[equipment_id]
        self.owner = owner_side
        
        # Unit state
        self.strength = 10  # HP, max 10
        self.experience = 0
        self.fuel = self.data["fuel"]
        self.ammo = self.data["ammo"]
        
        # Turn state
        self.has_moved = False
        self.has_fired = False
        self.move_left = self.data["move_points"]
        
        # Position
        self.hex = None
        
    def get_name(self):
        """Get unit display name"""
        return self.data["name"]
    
    def get_class(self):
        """Get unit class"""
        return self.data["unit_class"]
    
    def get_type(self):
        """Get unit type"""
        return self.data["unit_type"]
    
    def get_attack(self):
        """Get current attack value"""
        # Attack scales with strength and experience
        base_attack = self.data["attack"]
        strength_modifier = self.strength / 10.0
        exp_modifier = 1.0 + (self.experience / 100.0)
        return int(base_attack * strength_modifier * exp_modifier)
    
    def get_defense(self):
        """Get current defense value"""
        base_defense = self.data["defense"]
        strength_modifier = self.strength / 10.0
        exp_modifier = 1.0 + (self.experience / 100.0)
        
        # Add terrain bonus
        terrain_bonus = 0
        if self.hex:
            terrain_bonus = self.hex.get_defense_bonus()
        
        defense = int(base_defense * strength_modifier * exp_modifier * (1 + terrain_bonus / 100.0))
        return defense
    
    def get_moves_left(self):
        """Get remaining movement points"""
        return self.move_left
    
    def can_move(self):
        """Check if unit can still move this turn"""
        return not self.has_moved and self.move_left > 0 and self.fuel > 0
    
    def can_attack(self):
        """Check if unit can attack this turn"""
        return not self.has_fired and self.ammo > 0 and self.strength > 0
    
    def move_to(self, hex_tile, cost):
        """Move unit to new hex"""
        if self.hex:
            self.hex.unit = None
        
        self.hex = hex_tile
        hex_tile.unit = self
        
        self.fuel -= cost
        self.move_left -= cost
        
        if self.move_left <= 0:
            self.has_moved = True
    
    def attack(self, defender):
        """
        Attack another unit.
        Returns (attacker_losses, defender_losses)
        """
        if self.ammo <= 0:
            return (0, 0)
        
        self.ammo -= 1
        self.has_fired = True
        
        # Combat calculation based on attack vs defense
        attacker_power = self.get_attack()
        defender_power = defender.get_defense()
        
        # Losses calculation (simplified from original)
        # Defender takes more damage
        defender_losses = max(1, (attacker_power - defender_power // 2) // 3)
        # Attacker takes some return fire
        attacker_losses = max(0, (defender_power - attacker_power // 2) // 4)
        
        # Apply losses
        defender.take_damage(defender_losses)
        self.take_damage(attacker_losses)
        
        # Gain experience
        self.experience += 5
        if defender.strength <= 0:
            self.experience += 10
        
        return (attacker_losses, defender_losses)
    
    def take_damage(self, damage):
        """Take damage, reducing strength"""
        self.strength -= damage
        if self.strength < 0:
            self.strength = 0
    
    def is_destroyed(self):
        """Check if unit is destroyed"""
        return self.strength <= 0
    
    def resupply(self):
        """Resupply unit (partial)"""
        self.ammo = min(self.ammo + 3, self.data["ammo"])
        self.fuel = min(self.fuel + 10, self.data["fuel"])
        self.has_moved = True
        self.has_fired = True
    
    def reinforce(self, amount=1):
        """Reinforce unit, restoring strength"""
        self.strength = min(self.strength + amount, 10)
        self.has_moved = True
        self.has_fired = True
    
    def end_turn(self):
        """Reset unit state for new turn"""
        self.move_left = self.data["move_points"]
        self.has_moved = False
        self.has_fired = False
    
    def get_info_string(self):
        """Get unit information as string"""
        return (f"{self.get_name()} (Str: {self.strength}/10)\n"
                f"Fuel: {self.fuel} | Ammo: {self.ammo}\n"
                f"Attack: {self.get_attack()} | Defense: {self.get_defense()}\n"
                f"Moves: {self.move_left}/{self.data['move_points']}")
    
    def __repr__(self):
        return f"Unit({self.get_name()}, Owner={self.owner}, Str={self.strength})"
