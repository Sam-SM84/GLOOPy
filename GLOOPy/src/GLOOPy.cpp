// =============================================================================
//  CRYSTAL RUSH  -  a small first-person arcade game built on OOGL
//
//  Collect every crystal before the timer runs out. Red chasers hunt you, and
//  they get faster with every crystal you grab. Jump over them, use the low
//  amber pillars as platforms, and watch your stamina when you sprint.
//
//  Controls:  WASD move | Mouse look | Space jump | Left Shift sprint
//             R restart | Esc quit
//
//  Files needed (relative to the working directory, like your other code):
//      shaders/claude.vs, shaders/claude.fs, shaders/6.1.skybox.vs/.fs,
//      images/skybox/*.jpg, models/Tree/Tree.obj (optional scenery)
//
//  Nothing in the OOGL repo has to change: every extra helper lives in this file.
// =============================================================================
#define STB_IMAGE_IMPLEMENTATION

#include "Buffer.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Shader.h"
#include "Skybox.h"
#include "Texture.h"
#include "FrameTime.h"
#include "Transform.h"
#include "VertexArray.h"
#include "Window.h"
#include "Light.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>
// -----------------------------------------------------------------------------
//  Tuning
// -----------------------------------------------------------------------------
namespace cfg
{
    const unsigned int SCR_W = 1280;
    const unsigned int SCR_H = 720;

    const float ARENA = 22.0f;   // half-size of the square arena
    const float WALL_H = 3.0f;
    const float PLAYER_R = 0.35f;
    const float EYE_H = 1.6f;
    const float STEP = 0.3f;    // how far below a ledge you can be and still mantle onto it

    const float GRAVITY = 24.0f;
    const float JUMP_V = 8.0f;    // apex ~1.33 units
    const float WALK = 5.0f;
    const float RUN = 8.5f;

    const int   CRYSTALS = 12;
    const int   MAX_LIVES = 3;
    const float TIME_LIMIT = 90.0f;
    const float TIME_BONUS = 3.0f;   // seconds added per crystal
}

// -----------------------------------------------------------------------------
//  Small helpers that OOGL does not have (kept here on purpose)
// -----------------------------------------------------------------------------
static void setVec2(Shader& s, const char* name, glm::vec2 v)
{
    glUniform2f(glGetUniformLocation(s.ID, name), v.x, v.y);
}

static float frand(std::mt19937& rng, float a, float b)
{
    return std::uniform_real_distribution<float>(a, b)(rng);
}

// A unit cube centred on the origin (size 1), with normals + UVs.
static Mesh makeBoxMesh()
{
    struct Face { glm::vec3 n, u, v; };            // u x v = n  ->  counter-clockwise from outside
    const Face faces[6] = {
        { { 0, 0, 1}, { 1, 0, 0}, {0, 1, 0} },
        { { 0, 0,-1}, {-1, 0, 0}, {0, 1, 0} },
        { { 1, 0, 0}, { 0, 0,-1}, {0, 1, 0} },
        { {-1, 0, 0}, { 0, 0, 1}, {0, 1, 0} },
        { { 0, 1, 0}, { 1, 0, 0}, {0, 0,-1} },
        { { 0,-1, 0}, { 1, 0, 0}, {0, 0, 1} },
    };
    const float su[4] = { -1,  1, 1, -1 };
    const float sv[4] = { -1, -1, 1,  1 };
    const glm::vec2 uvs[4] = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    std::vector<Vertex> verts;
    std::vector<unsigned int> idx;
    for (const Face& f : faces)
    {
        unsigned int base = (unsigned int)verts.size();
        for (int i = 0; i < 4; ++i)
        {
            Vertex v;
            v.position = f.n * 0.5f + f.u * (su[i] * 0.5f) + f.v * (sv[i] * 0.5f);
            v.normal = f.n;
            v.textureCoordinate = uvs[i];
            verts.push_back(v);
        }
        idx.insert(idx.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }
    return Mesh(verts, idx, std::vector<Texture>());
}

// A quad spanning -1..1 in x/y. Used for the HUD and the screen flash.
static Mesh makeQuadMesh()
{
    std::vector<Vertex> verts(4);
    const glm::vec2 p[4] = { {-1, -1}, {1, -1}, {1, 1}, {-1, 1} };
    for (int i = 0; i < 4; ++i)
    {
        verts[i].position = glm::vec3(p[i], 0.0f);
        verts[i].normal = glm::vec3(0, 0, 1);
        verts[i].textureCoordinate = glm::vec2((p[i].x + 1) * 0.5f, (p[i].y + 1) * 0.5f);
    }
    std::vector<unsigned int> idx = { 0, 1, 2, 0, 2, 3 };
    return Mesh(verts, idx, std::vector<Texture>());
}

// -----------------------------------------------------------------------------
//  Game state
// -----------------------------------------------------------------------------
enum class State { Playing, Won, Lost };

struct Solid
{
    enum Kind { Wall, Pillar, Tree } kind = Wall;
    glm::vec3 center{ 0.0f };
    glm::vec3 half{ 0.5f };
    glm::vec3 color{ 0.5f };
    float scale = 1.0f;     // trees only
    float yaw = 0.0f;       // trees only
    float top() const { return center.y + half.y; }
};

struct Crystal { glm::vec3 pos; float phase; bool taken; };
struct Enemy { glm::vec2 pos{ 0.0f }; float yaw = 0.0f; float phase = 0.0f; bool active = false; };
struct Input { float forward = 0.0f; float strafe = 0.0f; bool jump = false; bool sprint = false; };

class Game
{
public:
    State state = State::Playing;

    glm::vec3 pos{ 0.0f };       // feet position
    glm::vec2 vel{ 0.0f };       // horizontal velocity (x, z)
    float vy = 0.0f;
    bool  onGround = true;
    bool  sprinting = false;

    float stamina = 100.0f;
    bool  sprintLocked = false;
    int   lives = cfg::MAX_LIVES;
    int   collected = 0;
    float timeLeft = cfg::TIME_LIMIT;
    float invuln = 0.0f;

    float flash = 0.0f;
    glm::vec3 flashColor{ 1.0f, 0.0f, 0.0f };

    float clock = 0.0f;
    float bobPhase = 0.0f;
    float eyeBob = 0.0f;
    float fov = 70.0f;

    std::vector<Solid>   solids;
    std::vector<Crystal> crystals;
    std::vector<Enemy>   enemies;
    std::mt19937 rng;

    Game() : rng(std::random_device{}()) {}

    glm::vec3 eyePosition() const { return pos + glm::vec3(0.0f, cfg::EYE_H + eyeBob, 0.0f); }

    // ------------------------------------------------------------------ setup
    void reset(bool withTrees)
    {
        solids.clear();
        crystals.clear();
        enemies.clear();

        state = State::Playing;
        pos = glm::vec3(0.0f);
        vel = glm::vec2(0.0f);
        vy = 0.0f;
        onGround = true;
        sprinting = false;
        stamina = 100.0f;
        sprintLocked = false;
        lives = cfg::MAX_LIVES;
        collected = 0;
        timeLeft = cfg::TIME_LIMIT;
        invuln = 0.0f;
        flash = 0.0f;
        clock = 0.0f;
        bobPhase = 0.0f;
        eyeBob = 0.0f;
        fov = 70.0f;

        const float A = cfg::ARENA, T = 0.5f, H = cfg::WALL_H * 0.5f;
        const glm::vec3 stone(0.32f, 0.33f, 0.38f);
        addSolid(Solid::Wall, glm::vec3(0, H, -(A + T)), glm::vec3(A + 2 * T, H, T), stone);
        addSolid(Solid::Wall, glm::vec3(0, H, (A + T)), glm::vec3(A + 2 * T, H, T), stone);
        addSolid(Solid::Wall, glm::vec3(-(A + T), H, 0), glm::vec3(T, H, A + 2 * T), stone);
        addSolid(Solid::Wall, glm::vec3((A + T), H, 0), glm::vec3(T, H, A + 2 * T), stone);

        // pillars: low amber ones can be climbed, tall slate ones are walls
        const float heights[5] = { 0.9f, 1.2f, 1.5f, 2.6f, 3.4f };
        int placed = 0, tries = 0;
        while (placed < 14 && tries++ < 600)
        {
            glm::vec2 p(frand(rng, -A + 3, A - 3), frand(rng, -A + 3, A - 3));
            float hx = frand(rng, 0.8f, 1.5f), hz = frand(rng, 0.8f, 1.5f);
            if (glm::length(p) < 6.0f) continue;                     // keep the spawn clear
            if (overlapsSolid(p, std::max(hx, hz) + 0.8f)) continue;
            float h = heights[rng() % 5];
            glm::vec3 color = (h <= 1.5f) ? glm::vec3(0.85f, 0.55f, 0.18f) : glm::vec3(0.30f, 0.36f, 0.48f);
            addSolid(Solid::Pillar, glm::vec3(p.x, h * 0.5f, p.y), glm::vec3(hx, h * 0.5f, hz), color);
            ++placed;
        }

        if (withTrees)
        {
            placed = 0; tries = 0;
            while (placed < 8 && tries++ < 400)
            {
                glm::vec2 p(frand(rng, -A + 3, A - 3), frand(rng, -A + 3, A - 3));
                if (glm::length(p) < 7.0f) continue;
                if (overlapsSolid(p, 2.0f)) continue;
                float s = frand(rng, 0.9f, 1.3f);
                Solid t;
                t.kind = Solid::Tree;
                t.scale = s;
                t.yaw = frand(rng, 0.0f, 6.28f);
                t.half = glm::vec3(0.35f * s, 2.0f, 0.35f * s);
                t.center = glm::vec3(p.x, 2.0f, p.y);
                solids.push_back(t);
                ++placed;
            }
        }

        // crystals: every 4th one sits on top of a climbable pillar
        std::vector<int> low;
        for (int i = 0; i < (int)solids.size(); ++i)
            if (solids[i].kind == Solid::Pillar && solids[i].top() <= 1.5f) low.push_back(i);

        for (int i = 0; i < cfg::CRYSTALS; ++i)
        {
            Crystal c;
            c.phase = frand(rng, 0.0f, 6.28f);
            c.taken = false;
            if (i % 4 == 0 && !low.empty())
            {
                const Solid& s = solids[low[rng() % low.size()]];
                c.pos = glm::vec3(s.center.x, s.top() + 0.9f, s.center.z);
            }
            else
            {
                glm::vec2 p = freeSpot(1.2f, glm::vec2(0.0f), 6.0f, true);
                c.pos = glm::vec3(p.x, 0.9f, p.y);
            }
            crystals.push_back(c);
        }

        enemies.assign(4, Enemy());
        for (int i = 0; i < 2; ++i) spawnEnemy(enemies[i]);
    }

    // ----------------------------------------------------------------- update
    void update(float dt, const Input& in, const glm::vec3& fwd, const glm::vec3& right)
    {
        clock += dt;
        flash = std::max(0.0f, flash - dt * 1.4f);
        if (state != State::Playing) return;

        invuln = std::max(0.0f, invuln - dt);
        timeLeft -= dt;
        if (timeLeft <= 0.0f)
        {
            timeLeft = 0.0f;
            state = State::Lost;
            flashColor = glm::vec3(0.4f, 0.0f, 0.0f);
            return;
        }

        // ---- horizontal movement
        glm::vec3 wish = fwd * in.forward + right * in.strafe;
        float len = glm::length(wish);
        if (len > 1.0f) wish /= len;

        if (sprintLocked && stamina > 25.0f) sprintLocked = false;
        sprinting = in.sprint && len > 0.1f && !sprintLocked && stamina > 0.0f;
        if (sprinting)
        {
            stamina -= 30.0f * dt;
            if (stamina <= 0.0f) { stamina = 0.0f; sprintLocked = true; sprinting = false; }
        }
        else
        {
            stamina = std::min(100.0f, stamina + 18.0f * dt);
        }

        float speed = sprinting ? cfg::RUN : cfg::WALK;
        glm::vec2 target(wish.x * speed, wish.z * speed);
        float control = onGround ? 14.0f : 4.0f;
        vel += (target - vel) * (1.0f - std::exp(-control * dt));

        glm::vec2 p(pos.x, pos.z);
        p += vel * dt;
        resolveCircle(p, cfg::PLAYER_R, pos.y);
        pos.x = p.x;
        pos.z = p.y;

        // ---- vertical movement
        if (in.jump && onGround) { vy = cfg::JUMP_V; onGround = false; }
        vy -= cfg::GRAVITY * dt;
        float feetBefore = pos.y;
        pos.y += vy * dt;
        float ground = supportHeight(pos.x, pos.z, feetBefore);
        if (pos.y <= ground && vy <= 0.0f)
        {
            pos.y = ground;
            vy = 0.0f;
            onGround = true;
        }
        else
        {
            onGround = false;
        }

        // ---- camera feel
        float hs = glm::length(vel);
        if (onGround && hs > 0.5f)
        {
            bobPhase += hs * dt * 1.9f;
            eyeBob = std::sin(bobPhase) * 0.04f * std::min(hs / cfg::WALK, 1.5f);
        }
        else
        {
            eyeBob *= 0.9f;
        }
        float fovTarget = sprinting ? 79.0f : 70.0f;
        fov += (fovTarget - fov) * (1.0f - std::exp(-8.0f * dt));

        // ---- crystals
        glm::vec3 chest = pos + glm::vec3(0.0f, 0.9f, 0.0f);
        for (Crystal& c : crystals)
        {
            if (c.taken || glm::distance(chest, c.pos) > 1.15f) continue;
            c.taken = true;
            ++collected;
            timeLeft = std::min(cfg::TIME_LIMIT, timeLeft + cfg::TIME_BONUS);
            flash = std::max(flash, 0.35f);
            flashColor = glm::vec3(1.0f, 0.85f, 0.2f);
            if (collected >= cfg::CRYSTALS)
            {
                state = State::Won;
                flash = 0.6f;
                flashColor = glm::vec3(0.2f, 1.0f, 0.4f);
                return;
            }
        }

        // ---- enemies get more numerous and faster as you collect
        int want = std::min(4, 2 + collected / 4);
        for (int i = 0; i < want; ++i)
            if (!enemies[i].active) spawnEnemy(enemies[i]);

        float espeed = 3.0f + 0.3f * collected;
        glm::vec2 playerXZ(pos.x, pos.z);
        for (size_t i = 0; i < enemies.size(); ++i)
        {
            Enemy& e = enemies[i];
            if (!e.active) continue;

            glm::vec2 to = playerXZ - e.pos;
            float d = glm::length(to);
            if (d > 0.01f)
            {
                glm::vec2 dir = to / d;
                e.yaw = std::atan2(dir.x, dir.y);
                e.pos += dir * espeed * dt;
            }
            for (size_t j = 0; j < enemies.size(); ++j)           // don't stack on each other
            {
                if (j == i || !enemies[j].active) continue;
                glm::vec2 delta = e.pos - enemies[j].pos;
                float dd = glm::length(delta);
                if (dd < 1.2f && dd > 0.001f) e.pos += (delta / dd) * (1.2f - dd) * 0.5f;
            }
            resolveCircle(e.pos, 0.5f, 0.0f);
            e.pos = glm::clamp(e.pos, glm::vec2(-cfg::ARENA + 0.5f), glm::vec2(cfg::ARENA - 0.5f));
            e.phase += dt * 6.0f;

            // jump over them to stay safe
            if (invuln <= 0.0f && glm::length(playerXZ - e.pos) < cfg::PLAYER_R + 0.5f && pos.y < 0.75f)
            {
                glm::vec2 away = playerXZ - e.pos;
                if (glm::length(away) > 0.001f) vel = glm::normalize(away) * 9.0f;
                --lives;
                invuln = 2.0f;
                flash = 1.0f;
                flashColor = glm::vec3(1.0f, 0.05f, 0.05f);
                spawnEnemy(e);
                if (lives <= 0)
                {
                    state = State::Lost;
                    flashColor = glm::vec3(0.4f, 0.0f, 0.0f);
                    return;
                }
            }
        }
    }

private:
    void addSolid(Solid::Kind kind, glm::vec3 center, glm::vec3 half, glm::vec3 color)
    {
        Solid s;
        s.kind = kind;
        s.center = center;
        s.half = half;
        s.color = color;
        solids.push_back(s);
    }

    // is point p within `radius` of an existing pillar/tree footprint?
    bool overlapsSolid(glm::vec2 p, float radius) const
    {
        for (const Solid& s : solids)
        {
            if (s.kind == Solid::Wall) continue;
            glm::vec2 c(s.center.x, s.center.z);
            if (glm::length(p - c) < radius + std::max(s.half.x, s.half.z)) return true;
        }
        return false;
    }

    glm::vec2 freeSpot(float margin, glm::vec2 avoid, float minDist, bool awayFromCrystals)
    {
        glm::vec2 p(0.0f);
        for (int tries = 0; tries < 300; ++tries)
        {
            p = glm::vec2(frand(rng, -cfg::ARENA + 2, cfg::ARENA - 2), frand(rng, -cfg::ARENA + 2, cfg::ARENA - 2));
            if (glm::length(p - avoid) < minDist) continue;
            bool bad = false;
            for (const Solid& s : solids)
            {
                if (s.kind == Solid::Wall) continue;
                if (std::abs(p.x - s.center.x) < s.half.x + margin && std::abs(p.y - s.center.z) < s.half.z + margin) { bad = true; break; }
            }
            if (!bad && awayFromCrystals)
                for (const Crystal& c : crystals)
                    if (glm::length(p - glm::vec2(c.pos.x, c.pos.z)) < 3.0f) { bad = true; break; }
            if (!bad) return p;
        }
        return p;
    }

    void spawnEnemy(Enemy& e)
    {
        e.pos = freeSpot(1.0f, glm::vec2(pos.x, pos.z), 14.0f, false);
        e.yaw = 0.0f;
        e.phase = frand(rng, 0.0f, 6.28f);
        e.active = true;
    }

    // Highest surface under (x,z) that the feet can stand on.
    float supportHeight(float x, float z, float feetY) const
    {
        float h = 0.0f;
        for (const Solid& s : solids)
        {
            float top = s.top();
            if (top > feetY + cfg::STEP) continue;
            if (std::abs(x - s.center.x) <= s.half.x + cfg::PLAYER_R * 0.6f &&
                std::abs(z - s.center.z) <= s.half.z + cfg::PLAYER_R * 0.6f)
                h = std::max(h, top);
        }
        return h;
    }

    // Push a circle (x,z) out of every solid whose top is above the feet.
    void resolveCircle(glm::vec2& p, float r, float feetY) const
    {
        for (int iter = 0; iter < 3; ++iter)
        {
            for (const Solid& s : solids)
            {
                if (s.top() <= feetY + cfg::STEP) continue;     // low enough to stand on, not a wall
                glm::vec2 c(s.center.x, s.center.z), h(s.half.x, s.half.z);
                glm::vec2 closest = glm::clamp(p, c - h, c + h);
                glm::vec2 d = p - closest;
                float d2 = glm::dot(d, d);
                if (d2 >= r * r) continue;
                if (d2 > 1e-8f)
                {
                    float dist = std::sqrt(d2);
                    p += d / dist * (r - dist);
                }
                else                                            // centre is inside the box
                {
                    glm::vec2 q = p - c;
                    float px = h.x - std::abs(q.x), pz = h.y - std::abs(q.y);
                    if (px < pz) p.x += (q.x >= 0.0f ? 1.0f : -1.0f) * (px + r);
                    else         p.y += (q.y >= 0.0f ? 1.0f : -1.0f) * (pz + r);
                }
            }
        }
    }
};

// -----------------------------------------------------------------------------
//  Globals for GLFW callbacks
// -----------------------------------------------------------------------------
Camera camera(glm::vec3(0.0f, cfg::EYE_H, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
float lastX = cfg::SCR_W / 2.0f;
float lastY = cfg::SCR_H / 2.0f;
bool  firstMouse = true;
int   winW = cfg::SCR_W, winH = cfg::SCR_H;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    winW = width;
    winH = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }

    float xoffset = (xpos - lastX) * 0.1f;
    float yoffset = (lastY - ypos) * 0.1f;
    lastX = xpos;
    lastY = ypos;

    camera.yaw += xoffset;
    camera.pitch = std::max(-89.0f, std::min(89.0f, camera.pitch + yoffset));

    glm::vec3 front;
    front.x = std::cos(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch));
    front.y = std::sin(glm::radians(camera.pitch));
    front.z = std::sin(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch));
    camera.direction = glm::normalize(front);
}

// -----------------------------------------------------------------------------
//  Drawing helpers
// -----------------------------------------------------------------------------
struct Renderer
{
    Shader& shader;
    Mesh& box;
    Mesh& quad;
    Light light;

    void solid(const glm::mat4& model, glm::vec3 color, float emissive = 0.0f, bool checker = false)
    {
        shader.setMat4("model", model);
        shader.setVec3("uColor", color);
        shader.setFloat("uEmissive", emissive);
        shader.setBool("uChecker", checker);
        shader.setBool("uUseTexture", false);
        box.Draw(shader, light);
    }

    // rectangle in clip space (centre cx,cy, half-size hw,hh)
    void rect(float cx, float cy, float hw, float hh, glm::vec3 color, float alpha)
    {
        setVec2(shader, "uOverlayScale", glm::vec2(hw, hh));
        setVec2(shader, "uOverlayOffset", glm::vec2(cx, cy));
        shader.setVec3("uTint", color);
        shader.setFloat("uTintAmount", alpha);
        quad.Draw(shader, light);
    }
};

static glm::mat4 TRS(glm::vec3 t, float yawRad, glm::vec3 s)
{
    glm::mat4 m = glm::translate(glm::mat4(1.0f), t);
    m = glm::rotate(m, yawRad, glm::vec3(0, 1, 0));
    return glm::scale(m, s);
}

static void drawHUD(Renderer& r, const Game& g, float aspect)
{
    const float sq = 0.032f;                 // half-height of a HUD square (clip space)
    const float sqw = sq / aspect;           // matching half-width so they stay square
    const glm::vec3 gold(1.0f, 0.82f, 0.15f), grey(0.12f, 0.12f, 0.14f);

    // crystal progress (top-left)
    for (int i = 0; i < cfg::CRYSTALS; ++i)
    {
        bool got = i < g.collected;
        r.rect(-0.95f + sqw + i * sqw * 2.7f, 0.92f, sqw, sq, got ? gold : grey, got ? 0.95f : 0.55f);
    }

    // lives (top-right)
    for (int i = 0; i < cfg::MAX_LIVES; ++i)
    {
        bool alive = i < g.lives;
        r.rect(0.95f - sqw - i * sqw * 2.7f, 0.92f, sqw, sq,
            alive ? glm::vec3(0.95f, 0.15f, 0.2f) : grey, alive ? 0.95f : 0.55f);
    }

    // time bar (top-centre)
    float frac = std::max(0.0f, std::min(1.0f, g.timeLeft / cfg::TIME_LIMIT));
    r.rect(0.0f, 0.93f, 0.25f, 0.014f, grey, 0.6f);
    glm::vec3 tc = glm::mix(glm::vec3(0.95f, 0.15f, 0.1f), glm::vec3(0.2f, 0.9f, 0.4f), std::min(1.0f, frac * 2.0f));
    r.rect(-0.25f + 0.25f * frac, 0.93f, 0.25f * frac, 0.014f, tc, 0.95f);

    // stamina bar (bottom-centre)
    float sf = g.stamina / 100.0f;
    r.rect(0.0f, -0.92f, 0.18f, 0.01f, grey, 0.6f);
    r.rect(-0.18f + 0.18f * sf, -0.92f, 0.18f * sf, 0.01f,
        g.sprintLocked ? glm::vec3(1.0f, 0.5f, 0.1f) : glm::vec3(0.3f, 0.85f, 1.0f), 0.9f);

    // crosshair
    r.rect(0.0f, 0.0f, 0.018f / aspect, 0.0025f, glm::vec3(1.0f), 0.8f);
    r.rect(0.0f, 0.0f, 0.0025f / aspect, 0.018f, glm::vec3(1.0f), 0.8f);
}

// -----------------------------------------------------------------------------
//  main
// -----------------------------------------------------------------------------
int main()
{
    Window::initialize(3, 3);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    Window window(cfg::SCR_W, cfg::SCR_H, "Crystal Rush", NULL, NULL);
    window.screenSizeUpdaterFunction(framebuffer_size_callback);
    window.screenMouseMotionFunction(mouse_callback);
    glfwSetInputMode(window.screen, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader gameShader("shaders/claude.vs", "shaders/claude.fs");
    Shader skyboxShader("shaders/6.1.skybox.vs", "shaders/6.1.skybox.fs");

    std::vector<std::string> faces
    {
        "images/skybox/right.jpg", "images/skybox/left.jpg",
        "images/skybox/top.jpg",   "images/skybox/bottom.jpg",
        "images/skybox/front.jpg", "images/skybox/back.jpg"
    };
    Skybox skybox(faces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    stbi_set_flip_vertically_on_load(true);          // before the model textures are loaded
    Model tree("models/Tree/Tree.obj");
    const bool haveTrees = !tree.meshes.empty();

    Mesh boxMesh = makeBoxMesh();
    Mesh quadMesh = makeQuadMesh();

    // lightDirection points TOWARD the sun (same convention as OOGL's Light / test.fs)
    Light sun(glm::normalize(glm::vec3(0.45f, 1.0f, 0.30f)), glm::vec3(1.0f, 0.95f, 0.85f));
    Renderer r{ gameShader, boxMesh, quadMesh, sun };

    Time timeManager;                                // created after glfwInit on purpose
    Game game;
    game.reset(haveTrees);

    const glm::vec3 fogColor(0.45f, 0.50f, 0.55f);
    bool rWasDown = false;
    float titleTimer = 0.0f;

    while (!glfwWindowShouldClose(window.screen))
    {
        timeManager.update();
        float dt = std::min(timeManager.deltaTime, 0.05f);   // first frame / window drags can spike

        // ------------------------------------------------------------ input
        GLFWwindow* w = window.screen;
        if (glfwGetKey(w, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(w, true);

        Input in;
        if (glfwGetKey(w, GLFW_KEY_W) == GLFW_PRESS) in.forward += 1.0f;
        if (glfwGetKey(w, GLFW_KEY_S) == GLFW_PRESS) in.forward -= 1.0f;
        if (glfwGetKey(w, GLFW_KEY_D) == GLFW_PRESS) in.strafe += 1.0f;
        if (glfwGetKey(w, GLFW_KEY_A) == GLFW_PRESS) in.strafe -= 1.0f;
        in.jump = glfwGetKey(w, GLFW_KEY_SPACE) == GLFW_PRESS;
        in.sprint = glfwGetKey(w, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        bool rDown = glfwGetKey(w, GLFW_KEY_R) == GLFW_PRESS;
        if (rDown && !rWasDown) game.reset(haveTrees);
        rWasDown = rDown;

        glm::vec3 fwd = glm::normalize(glm::vec3(camera.direction.x, 0.0f, camera.direction.z));
        glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3(0.0f, 1.0f, 0.0f)));

        game.update(dt, in, fwd, right);
        camera.position = game.eyePosition();

        // ----------------------------------------------------------- window title (no text renderer in OOGL)
        titleTimer -= dt;
        if (titleTimer <= 0.0f)
        {
            titleTimer = 0.1f;
            char buf[160];
            const char* msg = game.state == State::Won ? "  |  YOU WIN!  Press R to play again"
                : game.state == State::Lost ? "  |  GAME OVER  Press R to retry" : "";
            std::snprintf(buf, sizeof(buf), "Crystal Rush  |  Crystals %d/%d  |  Lives %d  |  Time %.0fs%s",
                game.collected, cfg::CRYSTALS, game.lives, game.timeLeft, msg);
            glfwSetWindowTitle(w, buf);
        }

        // ------------------------------------------------------------ render
        window.beginFrame(0.45f, 0.5f, 0.55f, 1.0f, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float aspect = (winH > 0) ? (float)winW / (float)winH : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(game.fov, aspect, 0.1f, 150.0f);
        glm::mat4 view = camera.getViewMatrix();

        gameShader.use();
        gameShader.setMat4("projection", projection);
        gameShader.setMat4("view", view);
        gameShader.setVec3("uViewPos", camera.position);
        gameShader.setVec3("uFogColor", fogColor);
        gameShader.setFloat("uFogDensity", 0.016f);
        gameShader.setBool("uOverlay", false);
        sun.apply(gameShader);

        // ground
        const float G = cfg::ARENA * 2.0f + 4.0f;
        r.solid(TRS(glm::vec3(0, -0.1f, 0), 0.0f, glm::vec3(G, 0.2f, G)), glm::vec3(0.30f, 0.52f, 0.26f), 0.0f, true);

        // walls + pillars (trees are drawn as models below)
        for (const Solid& s : game.solids)
        {
            if (s.kind == Solid::Tree) continue;
            r.solid(TRS(s.center, 0.0f, s.half * 2.0f), s.color);
        }

        // crystals with a beacon so they can be spotted across the arena
        for (const Crystal& c : game.crystals)
        {
            if (c.taken) continue;
            float bob = std::sin(game.clock * 2.0f + c.phase) * 0.12f;
            glm::mat4 m = glm::translate(glm::mat4(1.0f), c.pos + glm::vec3(0, bob, 0));
            m = glm::rotate(m, game.clock * 1.5f + c.phase, glm::vec3(0, 1, 0));
            m = glm::rotate(m, glm::radians(45.0f), glm::normalize(glm::vec3(1, 0, 1)));
            m = glm::scale(m, glm::vec3(0.42f));
            r.solid(m, glm::vec3(1.0f, 0.82f, 0.15f), 1.0f);
            r.solid(TRS(c.pos + glm::vec3(0, 2.5f, 0), 0.0f, glm::vec3(0.07f, 5.0f, 0.07f)),
                glm::vec3(1.0f, 0.9f, 0.4f), 1.0f);
        }

        // chasers: red body + two glowing eyes that face you
        for (const Enemy& e : game.enemies)
        {
            if (!e.active) continue;
            float hop = std::abs(std::sin(e.phase)) * 0.12f;
            glm::mat4 base = glm::translate(glm::mat4(1.0f), glm::vec3(e.pos.x, 0.45f + hop, e.pos.y));
            base = glm::rotate(base, e.yaw, glm::vec3(0, 1, 0));
            r.solid(glm::scale(base, glm::vec3(0.9f)), glm::vec3(0.80f, 0.08f, 0.10f));
            for (int side = -1; side <= 1; side += 2)
            {
                glm::mat4 eye = glm::translate(base, glm::vec3(0.2f * side, 0.12f, 0.46f));
                r.solid(glm::scale(eye, glm::vec3(0.17f, 0.17f, 0.08f)), glm::vec3(1.0f, 1.0f, 0.85f), 1.0f);
            }
        }

        // trees (OOGL Model, textured)
        if (haveTrees)
        {
            gameShader.setBool("uUseTexture", true);
            gameShader.setBool("uChecker", false);
            gameShader.setFloat("uEmissive", 0.0f);
            for (const Solid& s : game.solids)
            {
                if (s.kind != Solid::Tree) continue;
                tree.transform.modelMatrix = TRS(glm::vec3(s.center.x, 0.0f, s.center.z), s.yaw, glm::vec3(s.scale));
                tree.draw(gameShader, sun);
            }
        }

        // skybox last, exactly like the original main.cpp
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", projection);
        skybox.drawSkybox(skyboxShader);

        // HUD + flashes on top
        gameShader.use();
        glDisable(GL_DEPTH_TEST);
        gameShader.setBool("uOverlay", true);
        if (game.flash > 0.0f)
            r.rect(0.0f, 0.0f, 1.0f, 1.0f, game.flashColor, std::min(1.0f, game.flash) * 0.45f);
        if (game.state == State::Won)  r.rect(0.0f, 0.0f, 1.0f, 1.0f, glm::vec3(0.1f, 0.6f, 0.25f), 0.25f);
        if (game.state == State::Lost) r.rect(0.0f, 0.0f, 1.0f, 1.0f, glm::vec3(0.35f, 0.0f, 0.0f), 0.5f);
        drawHUD(r, game, aspect);
        gameShader.setBool("uOverlay", false);
        glEnable(GL_DEPTH_TEST);

        window.endFrame();
    }

    glfwTerminate();
    return 0;
}
