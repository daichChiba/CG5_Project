#include "KamataEngine.h"
using namespace KamataEngine;
bool Input::ReleseKey(BYTE keyNumber) const {
	if (!Input::key_[keyNumber] && Input::keyPre_[keyNumber]) {
		return true;
	}
	return false;
}
bool Input::IsReleseMouse(int32_t mouseNumber) const {
	if (!Input::mouse_.rgbButtons[mouseNumber] && Input::mousePre_.rgbButtons[mouseNumber]) {
		return true;
	}
	return false;
}