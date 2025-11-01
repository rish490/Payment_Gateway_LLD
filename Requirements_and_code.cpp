2. Functional Requirements
✅ Actors

Payer (User1) — initiates the payment

Payee (User2) — receives the payment

Payment Gateway — handles routing, validation, and coordination

Bank — processes debit/credit operations for users

✅ Key Flows

Payer selects payment type (UPI, card, etc.)

Payer selects another user (payee) and enters amount

PaymentGateway validates payer/payee and basic rules

PaymentGateway delegates debit to payer’s bank

PaymentGateway then delegates credit to payee’s bank

If any step fails, rollback and refund are triggered

Return result (success or failure) to user

Entity Diagram 

User
 ├── id : string
 ├── name : string
 ├── bankAccount : BankAccount*
 └── void initiatePayment(User* payee, double amount, PaymentGateway* gateway)

BankAccount
 ├── accountId : string
 ├── balance : double
 ├── Bank* bank
 ├── void debit(double amount)
 └── void credit(double amount)

Bank
 ├── id : string
 ├── map<string, BankAccount*> accounts
 ├── bool processDebit(BankAccount*, double)
 └── bool processCredit(BankAccount*, double)

PaymentGateway
 ├── bool validateUsers(User* payer, User* payee, double amount)
 ├── bool processPayment(User* payer, User* payee, double amount)

PaymentManager
 ├── PaymentGateway* gateway
 └── void initiatePayment(User* payer, User* payee, double amount)

PaymentIntent
 ├── id : string
 ├── payerId : string
 ├── payeeId : string
 ├── amount : double
 ├── status : string ("PENDING", "SUCCESS", "FAILED")


Code (Basic Brute)

class BankAccount {
public:
    string accountId;
    double balance;
    Bank* bank;

    bool debit(double amount) {
        if (balance < amount) return false;
        balance -= amount;
        return true;
    }

    void credit(double amount) {
        balance += amount;
    }
};

class Bank {
public:
    string bankId;
    map<string, BankAccount*> accounts;

    bool processDebit(BankAccount* acc, double amount) {
        cout << "[Bank] Processing debit from " << acc->accountId << endl;
        return acc->debit(amount);
    }

    bool processCredit(BankAccount* acc, double amount) {
        cout << "[Bank] Processing credit to " << acc->accountId << endl;
        acc->credit(amount);
        return true;
    }
};

class User {
public:
    string id;
    string name;
    BankAccount* account;

    User(string id, string name, BankAccount* acc)
        : id(id), name(name), account(acc) {}

    void initiatePayment(User* payee, double amount, PaymentGateway* gateway) {
        cout << "\n[" << name << "] Initiating payment of " << amount << " to " << payee->name << endl;
        gateway->processPayment(this, payee, amount);
    }
};

class PaymentGateway {
public:
    bool validateUsers(User* payer, User* payee, double amount) {
        if (!payer || !payee) {
            cout << "[Gateway] Invalid users!" << endl;
            return false;
        }
        if (amount <= 0) {
            cout << "[Gateway] Invalid amount!" << endl;
            return false;
        }
        return true;
    }

    bool processPayment(User* payer, User* payee, double amount) {
        if (!validateUsers(payer, payee, amount)) return false;

        Bank* payerBank = payer->account->bank;
        Bank* payeeBank = payee->account->bank;

        cout << "[Gateway] Requesting debit from payer bank..." << endl;
        bool debitSuccess = payerBank->processDebit(payer->account, amount);

        if (!debitSuccess) {
            cout << "[Gateway] Payment failed: insufficient balance." << endl;
            return false;
        }

        cout << "[Gateway] Requesting credit to payee bank..." << endl;
        bool creditSuccess = payeeBank->processCredit(payee->account, amount);

        if (!creditSuccess) {
            // refund logic in simple terms
            cout << "[Gateway] Credit failed! Refunding payer..." << endl;
            payer->account->credit(amount);
            return false;
        }

        cout << "[Gateway] Payment successful!" << endl;
        return true;
    }
};

class PaymentManager {
public:
    PaymentGateway* gateway;

    PaymentManager(PaymentGateway* g) : gateway(g) {}

    void initiatePayment(User* payer, User* payee, double amount) {
        cout << "\n[Manager] Initiating transaction request..." << endl;
        gateway->processPayment(payer, payee, amount);
    }
};

// === DEMO ===
int main() {
    Bank bank1{"B1"}, bank2{"B2"};
    BankAccount acc1{"A1", 500, &bank1};
    BankAccount acc2{"A2", 200, &bank2};

    bank1.accounts["A1"] = &acc1;
    bank2.accounts["A2"] = &acc2;

    User user1{"U1", "Alice", &acc1};
    User user2{"U2", "Bob", &acc2};

    PaymentGateway gateway;
    PaymentManager manager(&gateway);

    manager.initiatePayment(&user1, &user2, 150);
    manager.initiatePayment(&user1, &user2, 400);  // should fail due to balance
}
