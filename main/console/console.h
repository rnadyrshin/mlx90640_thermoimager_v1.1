#ifndef MAIN_CONSOLE_CONSOLE_H_
#define MAIN_CONSOLE_CONSOLE_H_

typedef enum
{
	MsgInfo = 0,
	MsgWarning = 1,
	MsgError = 2
}
eConsoleMsgType;



// Процедура делает паузу в выводе консоли
void console_pause(uint32_t timeMs);
// Процедура добавляет новую строку в консоль
void console_printf(eConsoleMsgType msgType, const char *args, ...);
// Процедура выводит сообщение о перезагрузке в консоль и перезагружает esp32
void FatalError();
// Процедура выводит сообщение об ошибке и о перезагрузке в консоль и перезагружает esp32
void FatalErrorMsg(const char *args, ...);


#endif /* MAIN_CONSOLE_CONSOLE_H_ */
