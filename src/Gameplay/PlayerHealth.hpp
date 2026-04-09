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
    }

    void Reset()
    {
        health = maxHealth;
        isDead = false;
    }

    float GetPercent() const { return health / maxHealth; }
    int GetHealth() const { return static_cast<int>(health); }
};
#endif // !PLAYER_HEALTH_HPP_DEFINED
