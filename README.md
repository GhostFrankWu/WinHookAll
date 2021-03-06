# Hook掉大部分Windows的用户交互（需要管理员特权）

## 用途
禁用绝大部分用户和计算机的物理交互（键盘 鼠标 热键），并通过定时或按下指定热键恢复正常使用。  
有的懒人（比如说我）经常想用湿巾擦键盘鼠标，但是又不想关机，擦来擦去又会误触很多按键，就想找一个能暂时禁用掉用户交互的办法，于是就有了这段代码。   

## 食用方法
- 下载[Release版本](TODO://ADD_Release)  

**或者**
- 下载代码 使用VisualStudio编译 我的编译选项是**x86 Release**  

## 原理
低级钩子吃掉鼠标和键盘消息
```cpp
g_Hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);
g_MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, hInstance, 0);
```
挂起winlogin，解决一些键盘屏蔽干不掉的热键
```cpp
ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, FindProcessId(L"winlogon.exe"));
```

按下通过指定组合键可以恢复（默认为同时按下左右shift）  
超时后按任意键恢复（在开头的宏定义中指定限制）

## 特性
| 特点 | 实现 |
-- | -- 
屏蔽鼠标 | √
屏蔽任意单个按键 | √
屏蔽ctrl + alt + del | √
屏蔽任意组合键 | 未验证
屏蔽开机键 | x
win+g | x
function | x
滑动触控板 | x
-- | -- 
按下指定键恢复 | √
定时恢复 | √

## todo
处理win+任意键的热键
处理触控设备
