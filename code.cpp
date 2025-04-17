
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept> // For standard exceptions

using namespace std;

// Stock Class
class Stock {
public:
    int stockID;
    string name;
    double price;
    int volume;

    Stock(int id, string n, double p, int v)
        : stockID(id), name(n), price(p), volume(v) {}

    void UpdatePrice(double newPrice) {
        price = newPrice;
    }

    void UpdateVolume(int change) {
        if (volume + change < 0) {
            throw runtime_error("Insufficient stock volume available.");
        }
        volume += change; // Negative change reduces volume, positive increases
    }

    void Display() {
        cout << "Stock ID: " << stockID << ", Name: " << name
             << ", Price: $" << price << ", Volume: " << volume << endl;
    }
};

// User Class
class User {
protected:
    vector<pair<int, int>> portfolio; // Pair of stockID and quantity
    double balance;

public:
    User(double bal) : balance(bal) {}

    virtual ~User() {} // Virtual destructor for proper cleanup

    void ViewPortfolio() {
        cout << "\nYour Portfolio:\n";
        if (portfolio.empty()) {
            cout << "Portfolio is empty.\n";
            return;
        }
        for (size_t i = 0; i < portfolio.size(); i++) {
            cout << "Stock ID: " << portfolio[i].first
                 << ", Quantity: " << portfolio[i].second << endl;
        }
        cout << "Available Balance: $" << balance << endl;
    }

    virtual bool Buy(Stock& stock, int quantity) {
        double cost = stock.price * quantity;
        try {
            if (balance < cost) {
                throw runtime_error("Insufficient balance.");
            }
            if (stock.volume < quantity) {
                throw runtime_error("Insufficient stock volume available.");
            }
            balance -= cost;
            stock.UpdateVolume(-quantity);

            // Update portfolio
            bool found = false;
            for (size_t i = 0; i < portfolio.size(); i++) {
                if (portfolio[i].first == stock.stockID) {
                    portfolio[i].second += quantity;
                    found = true;
                    break;
                }
            }
            if (!found) {
                portfolio.push_back(make_pair(stock.stockID, quantity));
            }

            cout << "Bought " << quantity << " shares of " << stock.name << endl;
            return true;
        } catch (const runtime_error& e) {
            cout << "Error: " << e.what() << endl;
            return false;
        }
    }

    virtual bool Sell(Stock& stock, int quantity) {
        try {
            for (size_t i = 0; i < portfolio.size(); i++) {
                if (portfolio[i].first == stock.stockID) {
                    if (portfolio[i].second < quantity) {
                        throw runtime_error("Not enough shares in portfolio to sell.");
                    }
                    double revenue = stock.price * quantity;
                    balance += revenue;
                    stock.UpdateVolume(quantity);
                    portfolio[i].second -= quantity;

                    if (portfolio[i].second == 0) {
                        portfolio.erase(portfolio.begin() + i);
                    }

                    cout << "Sold " << quantity << " shares of " << stock.name << endl;
                    return true;
                }
            }
            throw runtime_error("Stock not found in portfolio.");
        } catch (const runtime_error& e) {
            cout << "Error: " << e.what() << endl;
            return false;
        }
    }
};

// Broker Class
class Broker : public User {
    double commissionRate;

public:
    Broker(double bal, double rate) : User(bal), commissionRate(rate) {}

    bool Buy(Stock& stock, int quantity) override {
        double cost = stock.price * quantity;
        double commission = cost * commissionRate;
        try {
            if (balance < cost + commission) {
                throw runtime_error("Insufficient balance for purchase and commission.");
            }
            if (stock.volume < quantity) {
                throw runtime_error("Insufficient stock volume available.");
            }
            balance -= (cost + commission);
            stock.UpdateVolume(-quantity);

            // Update portfolio
            bool found = false;
            for (size_t i = 0; i < portfolio.size(); i++) {
                if (portfolio[i].first == stock.stockID) {
                    portfolio[i].second += quantity;
                    found = true;
                    break;
                }
            }
            if (!found) {
                portfolio.push_back(make_pair(stock.stockID, quantity));
            }

            cout << "Bought " << quantity << " shares of " << stock.name
                 << " with commission: $" << commission << endl;
            return true;
        } catch (const runtime_error& e) {
            cout << "Error: " << e.what() << endl;
            return false;
        }
    }

    bool Sell(Stock& stock, int quantity) override {
        try {
            for (size_t i = 0; i < portfolio.size(); i++) {
                if (portfolio[i].first == stock.stockID) {
                    if (portfolio[i].second < quantity) {
                        throw runtime_error("Not enough shares in portfolio to sell.");
                    }
                    double revenue = stock.price * quantity;
                    double commission = revenue * commissionRate;
                    balance += (revenue - commission);
                    stock.UpdateVolume(quantity);
                    portfolio[i].second -= quantity;

                    if (portfolio[i].second == 0) {
                        portfolio.erase(portfolio.begin() + i);
                    }

                    cout << "Sold " << quantity << " shares of " << stock.name
                         << " with commission: $" << commission << endl;
                    return true;
                }
            }
            throw runtime_error("Stock not found in portfolio.");
        } catch (const runtime_error& e) {
            cout << "Error: " << e.what() << endl;
            return false;
        }
    }
};

// Market Class
class Market {
    vector<Stock> stockList;

public:
    void AddStock(Stock stock) {
        stockList.push_back(stock);
    }

    void DisplayStocks() {
        cout << "\nMarket Stocks:\n";
        for (size_t i = 0; i < stockList.size(); i++) {
            stockList[i].Display();
        }
    }

    Stock* GetStockByID(int stockID) {
        for (size_t i = 0; i < stockList.size(); i++) {
            if (stockList[i].stockID == stockID) {
                return &stockList[i];
            }
        }
        return nullptr;
    }
};

int main() {
    try {
        Market market;
        market.AddStock(Stock(1, "AAPL", 150.0, 1000));
        market.AddStock(Stock(2, "GOOGL", 2800.0, 500));
        market.AddStock(Stock(3, "AMZN", 3500.0, 300));

        // Input for broker's balance and commission rate
        double initialBalance, commissionRate;
        cout << "Enter initial balance for the broker: $";
        cin >> initialBalance;
        if (initialBalance < 0) throw invalid_argument("Balance cannot be negative.");

        cout << "Enter commission rate for the broker (as a decimal, e.g., 0.02 for 2%): ";
        cin >> commissionRate;
        if (commissionRate < 0) throw invalid_argument("Commission rate cannot be negative.");

        User* user = new Broker(initialBalance, commissionRate); // Broker user with input balance and commission
        market.DisplayStocks();

        int choice;
        do {
            cout << "\n1. View Portfolio\n2. Buy Stock\n3. Sell Stock\n4. View Market Stocks\n5. Exit\nChoose an action: ";
            cin >> choice;

            switch (choice) {
            case 1:
                user->ViewPortfolio();
                break;

            case 2: {
                int stockID, quantity;
                cout << "Enter Stock ID to buy: ";
                cin >> stockID;
                cout << "Enter quantity: ";
                cin >> quantity;

                Stock* stock = market.GetStockByID(stockID);
                if (stock) {
                    user->Buy(*stock, quantity);
                } else {
                    cout << "Stock not found.\n";
                }
                break;
            }

            case 3: {
                int stockID, quantity;
                cout << "Enter Stock ID to sell: ";
                cin >> stockID;
                cout << "Enter quantity: ";
                cin >> quantity;

                Stock* stock = market.GetStockByID(stockID);
                if (stock) {
                    user->Sell(*stock, quantity);
                } else {
                    cout << "Stock not found.\n";
                }
                break;
            }

            case 4:
                market.DisplayStocks();
                break;

            case 5:
                cout << "Exiting...\n";
                break;

            default:
                cout << "Invalid choice.\n";
            }
        } while (choice != 5);

        delete user;
    } 
    catch (const exception& e) {
        cout << "Error: " << e.what() << endl;
    }

    return 0;
}