#pragma once

struct CombatData
{
	unsigned int damage = 0;
};

class IDamageable
{
public:
	virtual ~IDamageable() {}
	virtual void OnHit(const CombatData aHitData) = 0;

protected:

};