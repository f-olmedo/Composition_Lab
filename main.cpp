/*
Author: Fabian Olmedo
Date: October 26, 2026
Project: Inheritance & Composition
Purpose:  Using updated UML diagram as a guide, implement the transaction history feature.
*/

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <iomanip>
//support transaction timestamps
#include <ctime>
#include <sstream>

using namespace std;


// NEW: Returns the current date and time
string currentTimestamp() {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    ostringstream output;
    output << put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return output.str();
}

struct Transaction {
    string type;
    double amount;
    string timestamp;

    //Auto add a timestamp
    Transaction(const string& transactionType, double transactionAmount)
        : type(transactionType), amount(transactionAmount), timestamp(currentTimestamp()) {}
};

class BankAccount {
private:
    int accountNumber;
    string accountHolderName;
    // NEW FOR COMPOSITION LAB: Composition - each BankAccount owns a vector of Transaction objects.
    vector<Transaction> transactionHistory;

protected:
    double balance;

    // NEW: Derived classes can log transactions without directly accessing the private vector.
    void recordTransaction(const string& type, double amount) {
        transactionHistory.emplace_back(type, amount);
    }

public:
    BankAccount() : accountNumber(0), accountHolderName("Unknown"), balance(0.0) {}

    BankAccount(int number, const string& name, double startingBalance)
        : accountNumber(number), accountHolderName(name), balance(startingBalance) {}

    virtual ~BankAccount() = default;

    int getAccountNumber() const { return accountNumber; }
    string getAccountHolderName() const { return accountHolderName; }
    double getBalance() const { return balance; }

    void setAccountHolderName(const string& name) { accountHolderName = name; }

    // CHANGED: Record a deposit only after the balance has been updated successfully
    virtual void deposit(double amount) {
        if (amount <= 0) {
            cout << "Deposit amount must be greater than zero.\n";
            return;
        }
        balance += amount;
        // NEW: Save the deposit details and timestamp in this account's history
        transactionHistory.emplace_back("Deposit", amount);
        cout << "Deposit successful.\n";
    }

    // CHANGED: Keep withdrawal virtual so CheckingAccount can apply its fee and log its own withdrawal
    virtual void withdraw(double amount) {
        if (amount <= 0) {
            cout << "Withdrawal amount must be greater than zero.\n";
            return;
        }
        if (amount > balance) {
            cout << "Insufficient funds.\n";
            return;
        }
        balance -= amount;
        // NEW: Save the successful withdrawal in this account's history
        transactionHistory.emplace_back("Withdrawal", amount);
        cout << "Withdrawal successful.\n";
    }

    // NEW: Display every transaction stored for this account
    void printHistory() const {
        cout << "\nTransaction History for " << accountHolderName
             << " (Account #" << accountNumber << ")\n";
        cout << left << setw(24) << "Date and Time" << setw(24) << "Type" << "Amount\n";
        cout << string(58, '-') << '\n';
        if (transactionHistory.empty()) {
            cout << "No transactions recorded.\n";
            return;
        }
        for (const Transaction& transaction : transactionHistory) {
            cout << left << setw(24) << transaction.timestamp
                 << setw(24) << transaction.type
                 << '$' << fixed << setprecision(2) << transaction.amount << '\n';
        }
    }

    virtual void print() const {
        cout << "Account Number: " << accountNumber
             << " | Holder: " << accountHolderName
             << " | Balance: $" << fixed << setprecision(2) << balance << '\n';
    }

    BankAccount& operator+=(double amount) {
        deposit(amount);
        return *this;
    }

    BankAccount& operator-=(double amount) {
        withdraw(amount);
        return *this;
    }

    bool operator==(const BankAccount& other) const { return balance == other.balance; }
    bool operator<(const BankAccount& other) const { return balance < other.balance; }
    bool operator>(const BankAccount& other) const { return balance > other.balance; }
};

// Public inheritance gives checking accounts the common BankAccount interface
class CheckingAccount : public BankAccount {
private:
    double transactionFee;

public:
    CheckingAccount(int number, const string& name, double startingBalance, double fee)
        : BankAccount(number, name, startingBalance), transactionFee(fee) {}

    // CHANGED: The checking withdrawal subtracts the fee and logs the total amount removed.
    void withdraw(double amount) override {
        if (amount <= 0) {
            cout << "Withdrawal amount must be greater than zero.\n";
            return;
        }
        double total = amount + transactionFee;
        if (total > balance) {
            cout << "Insufficient funds to cover the withdrawal and transaction fee.\n";
            return;
        }
        balance -= total;
        // NEW: Use the protected helper so the checking withdrawal is included in the history.
        recordTransaction("Withdrawal (fee included)", total);
        cout << "Checking withdrawal successful. Fee: $" << fixed << setprecision(2)
             << transactionFee << '\n';
    }

    void print() const override {
        BankAccount::print();
        cout << "Account Type: Checking | Transaction Fee: $"
             << fixed << setprecision(2) << transactionFee << '\n';
    }
};

// Savings accounts reuse the base account fields and add an interest rate
class SavingsAccount : public BankAccount {
private:
    double interestRate;

public:
    SavingsAccount(int number, const string& name, double startingBalance, double rate)
        : BankAccount(number, name, startingBalance), interestRate(rate) {}

    // Existing savings feature retained; NEW: interest is also recorded in transaction history.
    void calculateInterest() {
        double interest = balance * interestRate;
        if (interest <= 0) {
            cout << "No interest to add.\n";
            return;
        }
        balance += interest;
        // NEW: Store interest as a transaction too, so the history reflects this balance change.
        recordTransaction("Interest", interest);
        cout << "Interest added: $" << fixed << setprecision(2) << interest << '\n';
    }

    void print() const override {
        BankAccount::print();
        cout << "Account Type: Savings | Interest Rate: "
             << fixed << setprecision(2) << interestRate * 100 << "%\n";
    }
};

// Input helper keeps asking until an integer is entered
int getInteger(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) return value;
        cout << "Invalid input. Please enter a whole number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Input helper keeps asking until a valid non negative amount is entered
double getAmount(const string& prompt) {
    double amount;
    while (true) {
        cout << prompt;
        if (cin >> amount && amount >= 0) return amount;
        cout << "Invalid amount. Enter a number that is zero or greater.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string getName(const string& prompt) {
    string name;
    cout << prompt;
    getline(cin >> ws, name);
    return name;
}

void displayAccounts(const vector<BankAccount*>& accounts) {
    if (accounts.empty()) {
        cout << "No accounts have been created.\n";
        return;
    }
    for (const BankAccount* account : accounts) account->print();
}


int main() {
    // CHANGED: The menu now includes transaction-history access for either derived account type
    vector<BankAccount*> accounts; // stores checking and savings accounts
    bool running = true;

    while (running) {
        cout << "\n===== Composition Lab: Bank Accounts =====\n"
             << "1. Create Checking Account\n"
             << "2. Create Savings Account\n"
             << "3. Display All Accounts\n"
             << "4. Deposit\n"
             << "5. Withdraw\n"
             // NEW MENU OPTION: View the selected account's transaction records
             << "6. Print Transaction History\n"
             << "7. Calculate Savings Interest\n"
             << "8. Change Account Holder Name\n"
             << "9. Compare Two Accounts\n"
             << "10. Exit\n";
        int choice = getInteger("Choose an option: ");

        if (choice == 1 || choice == 2) {
            int number = getInteger("Enter account number: ");
            string name = getName("Enter account holder name: ");
            double startingBalance = getAmount("Enter starting balance: $");
            if (choice == 1) {
                double fee = getAmount("Enter checking transaction fee: $");
                accounts.push_back(new CheckingAccount(number, name, startingBalance, fee));
                cout << "Checking account created.\n";
            } else {
                double rate = getAmount("Enter savings interest rate (e.g. 0.05 for 5%): ");
                accounts.push_back(new SavingsAccount(number, name, startingBalance, rate));
                cout << "Savings account created.\n";
            }
        } else if (choice == 3) {
            displayAccounts(accounts);
        } else if (choice >= 4 && choice <= 8) {
            if (accounts.empty()) {
                cout << "Create an account first.\n";
                continue;
            }
            for (size_t i = 0; i < accounts.size(); ++i) {
                cout << i + 1 << ". ";
                accounts[i]->print();
            }
            int selected = getInteger("Select account number from this list: ");
            if (selected < 1 || selected > static_cast<int>(accounts.size())) {
                cout << "Invalid selection.\n";
                continue;
            }
            BankAccount* account = accounts[selected - 1];
            if (choice == 4) {
                double amount = getAmount("Deposit amount: $");
                account->deposit(amount);
            } else if (choice == 5) {
                double amount = getAmount("Withdrawal amount: $");
                // CHANGED: Virtual dispatch calls CheckingAccount::withdraw when appropriate
                account->withdraw(amount);
            } else if (choice == 6) {
                // NEW: This works for both CheckingAccount and SavingsAccount through BankAccount
                account->printHistory();
            } else if (choice == 7) {
                SavingsAccount* savings = dynamic_cast<SavingsAccount*>(account);
                if (savings) savings->calculateInterest();
                else cout << "Interest can only be calculated for a savings account.\n";
            } else if (choice == 8) {
                string newName = getName("Enter new account holder name: ");
                account->setAccountHolderName(newName);
                cout << "Account holder name updated.\n";
            }
        } else if (choice == 9) {
            if (accounts.size() < 2) {
                cout << "Create at least two accounts to compare them.\n";
                continue;
            }
            for (size_t i = 0; i < accounts.size(); ++i) {
                cout << i + 1 << ". ";
                accounts[i]->print();
            }
            int first = getInteger("Select the first account: ");
            int second = getInteger("Select the second account: ");
            if (first < 1 || second < 1 || first > static_cast<int>(accounts.size()) ||
                second > static_cast<int>(accounts.size())) {
                cout << "Invalid selection.\n";
                continue;
            }
            BankAccount& a = *accounts[first - 1];
            BankAccount& b = *accounts[second - 1];
            if (a == b) cout << "The accounts have equal balances.\n";
            else if (a > b) cout << "The first account has the larger balance.\n";
            else cout << "The second account has the larger balance.\n";
        } else if (choice == 10) {
            running = false;
        } else {
            cout << "Invalid menu option.\n";
        }
    }

    // Delete each dynamically allocated account before the program exits
    for (BankAccount* account : accounts) delete account;
    cout << "Goodbye!\n";
    return 0;
}
