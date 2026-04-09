#ifndef PLAYER_HEALTH_HPP_DEFINED
#define PLAYER_HEALTH_HPP_DEFINED

struct PlayerHealthComponent
{
    float maxHealth = 100.0f;
    float health = 100.0f;
    bool  isDead = false;

    void TakeDamage(float _dmg)
    {
        if (isDead) return;
        health -= _dmg;
		std::cout << "Player took " << _dmg << " damage, health is now " << health << std::endl;
        if (health <= 0.0f)
        {
            health = 0.0f;
            isDead = true;
        }
    }

    void Heal(float _amount)
    {
        if (isDead) return;
        health += _amount;
        if (health > maxHealth) health = maxHealth;
		std::cout << "Player healed " << _amount << ", health is now " << health << std::endl;
    }

    void Reset()
    {
        health = maxHealth;
        isDead = false;
    }

    float GetPercent() const { return health / maxHealth; }
};
#endif // !PLAYER_HEALTH_HPP_DEFINED
