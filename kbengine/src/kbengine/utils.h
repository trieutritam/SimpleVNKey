/*
 * utils.h
 *
 *  Created on: Sep 24, 2022
 *      Author: tamtt5
 */
#ifndef KBENGINE_UTILS_H_
#define KBENGINE_UTILS_H_

#ifdef DEBUG
#define PRINT_VECTOR(v) { cout << "List: "; \
                            for (auto ii=v.begin(); ii != v.end(); ii++) cout << (unsigned short) ((*ii) ^ MASK_EXTRA_MARK) << ' '; \
                            cout << endl;}
#else
#define PRINT_VECTOR(v)
#endif

#define EXCLUDE_MARK(keycode) UInt8(keycode ^ MASK_EXTRA_MARK)

#define IS_VOWEL(keycode) (keycode == KEY_A || keycode == KEY_E || keycode == KEY_I || keycode == KEY_Y || keycode == KEY_O || keycode == KEY_U)


#endif /* KBENGINE_UTILS_H_ */
