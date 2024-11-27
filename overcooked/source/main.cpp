#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <string>
#include <framework.h>
#include <enum.h>
int main()
{
    std::ios::sync_with_stdio(false);
    std::cerr.tie(nullptr);
    std::cerr << std::nounitbuf;
    std::string s;
    std::stringstream ss;
    int frame;

    init_read();

    /*
        你可以在读入后进行一些相关预处理，时间限制：5秒钟
        init();
    */
    init();

    int totalFrame = 14400;
    for (int i = 0; i < totalFrame; i++)
    {
        bool skip = frame_read(i);
        if (skip) continue;

        /* 输出当前帧的操作，此处仅作示例 */
        std::cout << "Frame " << i << "\n";
        //std::string player0_Action = "Move D";
        //std::string player1_Action = "Move U";

        /* 合成一个字符串再输出，否则输出有可能会被打断 */
        //std::string action = player0_Action + "\n" + player1_Action + "\n";
        //std::cout << action;
        strategy();
        /* 不要忘记刷新输出流，否则游戏将无法及时收到响应 */
        std::cout.flush();
    }
}
//./QtOvercooked.exe -l ../overcooked/maps/level1/level1-1.txt -p ../overcooked/out/build/x64-Debug/main.exe
