#include <iostream>
#include <string>

// GOD CLASS - too many methods and fields
class OrderProcessor {
public:
    std::string customerName;
    std::string customerEmail;
    std::string shippingAddress;
    std::string billingAddress;
    std::string paymentMethod;
    double      orderTotal;
    int         itemCount;
    bool        isPriority;
    bool        isGift;
    std::string giftMessage;
    int         loyaltyPoints;
    std::string couponCode;
    double      discountAmount;
    bool        requiresSignature;
    std::string trackingNumber;
    int         estimatedDays;

    void processPayment()     {}
    void validateAddress()    {}
    void applyDiscount()      {}
    void sendConfirmation()   {}
    void notifyWarehouse()    {}
    void updateInventory()    {}
    void generateInvoice()    {}
    void scheduleDelivery()   {}
    void applyLoyaltyPoints() {}
    void checkFraud()         {}
    void logTransaction()     {}
    void notifyCustomer()     {}
    void handleRefund()       {}
    void archiveOrder()       {}
    void generateReport()     {}
    void syncWithCRM()        {}
    void updateDashboard()    {}
    void sendSMSAlert()       {}
    void applyTax()           {}
    void checkStock()         {}
    void reserveStock()       {}
    void releaseStock()       {}
};

// LONG FUNCTION - too many lines
int calculateShipping(int weight) {
    std::cout << "Step 1\n";  std::cout << "Step 2\n";
    std::cout << "Step 3\n";  std::cout << "Step 4\n";
    std::cout << "Step 5\n";  std::cout << "Step 6\n";
    std::cout << "Step 7\n";  std::cout << "Step 8\n";
    std::cout << "Step 9\n";  std::cout << "Step 10\n";
    std::cout << "Step 11\n"; std::cout << "Step 12\n";
    std::cout << "Step 13\n"; std::cout << "Step 14\n";
    std::cout << "Step 15\n"; std::cout << "Step 16\n";
    std::cout << "Step 17\n"; std::cout << "Step 18\n";
    std::cout << "Step 19\n"; std::cout << "Step 20\n";
    return weight * 350;
}

// MAGIC NUMBERS
double applyPricing(double price) {
    if (price > 100) return price * 0.85;
    return price * 0.95;
}

// DEEP NESTING - 5 levels
bool checkEligibility(bool a, bool b, bool c, bool d) {
    if (a) {
        if (b) {
            if (c) {
                if (d) {
                    if (true) { return true; }
                }
            }
        }
    }
    return false;
}

int main() {
    OrderProcessor op;
    op.processPayment();
    int cost      = calculateShipping(10);
    double price  = applyPricing(120.0);
    bool ok       = checkEligibility(true, true, true, true);
    std::cout << cost << price << ok << "\n";
    return 0;
}