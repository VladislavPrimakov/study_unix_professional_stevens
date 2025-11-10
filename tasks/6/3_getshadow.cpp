// Представьте, что система использует теневой файл паролей и вам необходимо получить пароль в зашифрованном виде. Как это можно сделать?

#include <apue.h>
#include <memory>
#include <shadow.h>

std::unique_ptr<struct spwd> getshadow(const std::string name) {
	struct spwd* entry_ptr;
	std::unique_ptr<struct spwd> ptr;
	setspent();
	while ((entry_ptr = getspent()) != nullptr) {
		if (entry_ptr->sp_namp != nullptr && name == entry_ptr->sp_namp) {
			ptr = std::make_unique<struct spwd>(*entry_ptr);
			break;
		}
	}
	endspent();
	return(ptr);
}

int main() {
	auto shadow_passwd = getshadow("root");
	if (shadow_passwd != NULL) {
		std::println("Shadow password for root: {}", shadow_passwd->sp_pwdp);
	}
	else {
		std::println("Failed to retrieve shadow password for root.");
	}
	return 0;
}