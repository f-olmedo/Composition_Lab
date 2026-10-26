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
    // Enter the interest rate as a decimal
    SavingsAccount(const string& number, const string& name,
                   double initialBalance, double rate)
        : BankAccount(number, name, initialBalance),
          interestRate(rate) {
    }

    // Calculate simple interest from the current balance and add it to the balance
    void calculateInterest() {
        if (interestRate < 0) {
            cout << "Interest rate cannot be negative.\n";
            return;
        }

        double interest = balance * interestRate;
        balance += interest;

        cout << fixed << setprecision(2);
        cout << "Interest added: $" << interest << '\n';
        cout << "New balance: $" << balance << '\n';
    }

    // Override print to show the savings account's interest rate
    void print() const override {
        cout << fixed << setprecision(2);
        cout << "Account Type: Savings Account\n";
        cout << "Account Number: " << getAccountNumber() << '\n';
        cout << "Account Holder: " << getAccountHolderName() << '\n';
        cout << "Balance: $" << getBalance() << '\n';
        cout << "Interest Rate: " << interestRate * 100 << "%\n";
    }
};

int getInteger() {
    int value;

    while (!(cin >> value)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return value;
}

double getNonNegativeAmount(const string& prompt) {
    double amount;

    cout << prompt;
    while (!(cin >> amount) || amount < 0) {
        cout << "Invalid amount. Enter a non-negative value: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return amount;
}

double getPositiveAmount(const string& prompt) {
    double amount;

    cout << prompt;
    while (!(cin >> amount) || amount <= 0) {
        cout << "Invalid amount. Enter a value greater than 0: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    return amount;
}

void readAccountDetails(string& number, string& name, double& balance) {
    cout << "Enter account number: ";
    cin >> number;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << "Enter account holder name: ";
    getline(cin, name);

    balance = getNonNegativeAmount("Enter initial balance: $");
}

void displayAccounts(const vector<BankAccount*>& accounts) {
    if (accounts.empty()) {
        cout << "No accounts available.\n";
        return;
    }

    for (size_t i = 0; i < accounts.size(); ++i) {
        cout << "\nAccount " << i + 1 << '\n';
        accounts[i]->print();
    }
}

int main() {
    // The vector stores base-class pointers to both account types
    vector<BankAccount*> accounts;
    int choice = 0;

    do {
        cout << "\n===== Bank Account Management System =====\n";
        cout << "1. Create Checking Account\n";
        cout << "2. Create Savings Account\n";
        cout << "3. Display Accounts\n";
        cout << "4. Deposit\n";
        cout << "5. Withdraw\n";
        cout << "6. Change Account Holder Name\n";
        cout << "7. Calculate Savings Interest\n";
        cout << "8. Compare Accounts\n";
        cout << "9. Exit\n";
        cout << "Enter your choice: ";

        choice = getInteger();

        if (choice == 1) {
            // Read common account information, then ask for the checking-only fee
            string number;
            string name;
            double initialBalance;
            double fee;

            readAccountDetails(number, name, initialBalance);
            fee = getNonNegativeAmount("Enter transaction fee: $");

            accounts.push_back(
                new CheckingAccount(number, name, initialBalance, fee)
            );
            cout << "Checking account created successfully.\n";
        }
        else if (choice == 2) {
            // Read common account information, then ask for the savings interest rate
            string number;
            string name;
            double initialBalance;
            double rate;

            readAccountDetails(number, name, initialBalance);
            rate = getNonNegativeAmount(
                "Enter interest rate as a decimal (0.05 means 5%): "
            );

            accounts.push_back(
                new SavingsAccount(number, name, initialBalance, rate)
            );
            cout << "Savings account created successfully.\n";
        }
        else if (choice == 3) {
            displayAccounts(accounts);
        }
        else if (choice == 4 || choice == 5 || choice == 6 || choice == 7) {
            if (accounts.empty()) {
                cout << "No accounts available.\n";
                continue;
            }

            displayAccounts(accounts);
            cout << "\nEnter account number (1-"
                 << accounts.size() << "): ";
            int index = getInteger();

            if (index < 1 || index > static_cast<int>(accounts.size())) {
                cout << "Invalid account number.\n";
                continue;
            }

            BankAccount* account = accounts[index - 1];

            if (choice == 4) {
                // The overloaded += operator handles deposits for either account type
                double amount = getPositiveAmount("Enter deposit amount: $");
                *account += amount;
            }
            else if (choice == 5) {
                double amount = getPositiveAmount("Enter withdrawal amount: $");

                // Polymorphism: the correct withdraw() runs for the actual account type
                account->withdraw(amount);
            }
            else if (choice == 6) {
                string newName;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Enter new account holder name: ";
                getline(cin, newName);

                account->setAccountHolderName(newName);
                cout << "Account holder name updated.\n";
            }
            else if (choice == 7) {
                // dynamic_cast checks whether the selected base pointer is a savings account.
                SavingsAccount* savings = dynamic_cast<SavingsAccount*>(account);
if (savings != nullptr) {
                    savings->calculateInterest();
                }
                else {
                    cout << "Interest can only be calculated for a savings account.\n";
                }
            }
        }
        else if (choice == 8) {
            // Keep the same account comparison from last
            if (accounts.size() < 2) {
                cout << "You need at least two accounts to compare.\n";
                continue;
            }

            displayAccounts(accounts);
            cout << "\nEnter first account number: ";
            int first = getInteger();
            cout << "Enter second account number: ";
            int second = getInteger();

            if (first < 1 || first > static_cast<int>(accounts.size()) ||
                second < 1 || second > static_cast<int>(accounts.size())) {
                cout << "Invalid account number.\n";
                continue;
            }

            BankAccount& account1 = *accounts[first - 1];
            BankAccount& account2 = *accounts[second - 1];

            if (account1 == account2) {
                cout << "The accounts have the same account number.\n";
            }
            else {
                cout << "The accounts have different account numbers.\n";
            }

            if (account1 < account2) {
                cout << "The first account has a lower balance.\n";
            }
            else if (account1 > account2) {
                cout << "The first account has a higher balance.\n";
            }
            else {
                cout << "The accounts have the same balance.\n";
            }
        }
        else if (choice == 9) {
            cout << "Thank you for using the Bank Account Management System.\n";
        }
        else {
            cout << "Invalid choice. Please select 1-9.\n";
        }

    } while (choice != 9);

    // The virtual destructor makes deleting derived objects through base pointers safe.
    for (BankAccount* account : accounts) {
        delete account;
    }
    accounts.clear();

    return 0;
}