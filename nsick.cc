#include <iostream>
#include <functional>
#include <fstream>
#include <ncurses.h>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <unistd.h>

#include "seasick.h"
#include "ib/config.h"
#include "minibus/driver/minibus_driver.h"
#include "minibus/io/key.h"
#include "minibus/widgets/close_on_key.h"
#include "minibus/widgets/list_select.h"
#include "minibus/widgets/text.h"
#include "minibus/widgets/text_entry.h"

using namespace std;
using namespace std::placeholders;
using namespace minibus;
using namespace ib;
using namespace seasick;

class SeasickWidget : public XNav, public Text {
 public:
	SeasickWidget() : XNav(), Text("seasick", ""),
			  _prompt(" > "), _history_pos(0) {
	}

        virtual int render(IDisplay* win) {
		string status = Logger::stringify("% % %", _xmin, _xcur,
						  _xmax);
		string vars;
		_csick.get_vars(&vars);
                win->write(11, 0, vars);
                win->write(10, 0, status);
                win->write(2, 0, _result);
                win->write(0, 0, _prompt + _text.c_str());
                win->move(0, _prompt.length() + _xcur);
                return 0;
        }

        virtual int close() {
                return 0;
        }

        virtual int keypress(const Key& key) {
		if (key.enter()) {
			if (_text == "quit") return -1;
			enter();
			return 1;
		}
		_enter = false;

                if (key.del()) del();
                else if (key.up()) up();
                else if (key.down()) down();
                else if (key.backspace()) backspace();
                else if (key.tab()) tab();
                else if (key.navigation()) {
			XNav::keypress(key);
		}
                else if (key.key() >= 0 && key.key() < 256) {
                        insert(key, _xcur);
                }
                else return 0;
                return 1;
        }

protected:

	virtual void add_history(const string& history) {
		if (_history_pos == _history.size()) ++_history_pos;
		_history.push_back(history);
	}

        virtual void insert(const Key& key, size_t pos) {
                _text = _text.substr(0, pos)
                    + (char) key.key()
                    + ((pos < _text.length()) ? _text.substr(pos) : "");
                ++_xcur;
                ++_xmax;
        }

	virtual void set_text(const string& text) {
		_text = "";
		resize(_text.size());
	}

	virtual void enter() {
		if (_enter == true) {
			_enter = false;
			add_history(_text);
			set_text("");
		} else {
			string error;
			try {
				_csick.process(_text, &_result);
			} catch (string s) {
				error = s;
			} catch (const char* s) {
				error = s;
			}
			if (!error.empty()) {
				_result = error;
			}
			_enter = true;
		}
	}

	virtual void up() {
		if (_history_pos > 0) --_history_pos;
		//
	}

	virtual void down() {
		if (_history_pos < _history.size()) ++_history_pos;
		//
	}

	virtual void tab() {
		cout << _text;
		//
	}

        virtual void del() {
                if (_xcur < _text.size() - 1) {
                        cut(_xcur);
                }
        }

        virtual void backspace() {
                if (_xcur) {
                        --_xcur;
                        cut(_xcur);
                }
        }

        virtual void cut(size_t pos) {
                _text = _text.substr(0, pos) + _text.substr(pos + 1);
                --_xmax;
        }

        string _prompt;
	string _result;
	vector<string> _history;
	size_t _history_pos;
	Seasick _csick;
	bool _enter;
};

int main(int argc, char** argv) {
	Config::_()->load("csick.cfg");
	initscr();
	start_color();
	keypad(stdscr, true);
	noecho();
	cbreak();

	SeasickWidget* sw = new SeasickWidget();

	MinibusDriver md;
	md.start(sw);
	md.wait();
}

