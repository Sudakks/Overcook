#ifndef FRAMEWORK
#define FRAMEWORK

#include <string>
#include <vector>
#include <enum.h>

struct Ingredient
{
    int x, y, price;
    std::string name;
};

struct Recipe
{
    int time;
    std::string nameBefore, nameAfter;
    std::string kind;
};

struct Order
{
    int validFrame;
    int price;
    int frequency;
    std::vector<std::string> recipe;
};

struct Player
{
    double x, y;
    double X_Velocity;
    double Y_Velocity;
    int live;
    ContainerKind containerKind;
    std::vector<std::string> entity;
};

struct Entity
{
    double x, y;
    ContainerKind containerKind;
    std::vector<std::string> entity;
    int currentFrame, totalFrame;
    int sum;
};


/* 初始化时的读入。 */
void init_read();

/* 每一帧的读入；返回：是否跳帧。 */
bool frame_read(int nowFrame);


void strategy();
void init();
bool Move(int x, int y, double px, double py, int which);
void PutOrPick(int x1, int y1, int px, int py, int which);
void Interact(int x1, int y1, int px, int py, int which);
int check_clean_plate();
std::pair<int, int> find_location(char ch);
void send_menu_step(int which);
void get_item(std::string rec_name, int which);
bool check_line(double x1, double y1);
void assign(int which);
void _take_dirtyplate(int which);
void _clean_dirtyplate(int which);
void _cleanning_now(int which);
void judge_cook_kind(int which, int item);
void _find_ingredient(int which);
void _find_clean_plate(int which);
void _send_recipe(int which);
void _cook_chop(int which);
void _chopping_now(int which);
void _cook_pot_or_pan(int which);
#endif