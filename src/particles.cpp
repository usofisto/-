#include "particles.h"

const int MAX_PARTICLES = 120;
Particle particles[MAX_PARTICLES];

void InitParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        particles[i].active = false;
    }
}

void SpawnParticle(Vector2 position, Vector2 velocity, Color color, float size, float lifetime) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active) {
            particles[i].position = position;
            particles[i].velocity = velocity;
            particles[i].color = color;
            particles[i].size = size;
            particles[i].lifetime = lifetime;
            particles[i].maxLifetime = lifetime;
            particles[i].active = true;
            break;
        }
    }
}

void UpdateAndDrawParticles(float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].position.x += particles[i].velocity.x * dt;
            particles[i].position.y += particles[i].velocity.y * dt;
            particles[i].lifetime -= dt;
            
            float alpha = particles[i].lifetime / particles[i].maxLifetime;
            if (particles[i].lifetime <= 0.0f) {
                particles[i].active = false;
            } else {
                DrawCircleV(particles[i].position, particles[i].size * alpha, ColorAlpha(particles[i].color, alpha));
            }
        }
    }
}
