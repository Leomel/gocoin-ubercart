Version 1.5

�2014 GoCoin Holdings Limited and GoCoin International Group of companies hereby grants you permission to utilize a copy of this software and documentation in connection with your use of the GoCoin.com service subject the the published Terms of Use and Privacy Policy published on the site and subject to change from time to time at the discretion of GoCoin.<br><br>

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE DEVELOPERS OR AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.<br><br>

## Using the Official GoCoin Ubercart Plugin
When a shopper chooses the GoCoin payment method and places their order, they will be redirected to gateway.GoCoin.com to pay.  
GoCoin will send a notification to your server which this plugin handles.  Then the customer will be redirected to an order summary page.  

The order status in the admin panel will be "Payment received" when the order is placed and payment has been confirmed. 

#### Important Note: 
This plugin now supports Bitcoin,Dogecoin and Litecoin as well as Dogecoin

### 1. Installation
[Drupal](https://drupal.org/) and the [Ubercart extension](http://www.ubercart.org/) must be installed before installing this plugin.

a. 	download the code folder <b>uc_gocoin</b> and copy the folder and its contents to <<drupalroot>>/sites/all/modules/  Or also can be copied directly to <<drupalroot>>/modules/ubercart/payment/ folder.

### 2. Get API Key.
1) [Enable the GoCoin Hosted Payment Gateway](http://www.gocoin.com/docs/hosted_gateway)<br>
2) Get API Key from [GoCoin Dashboard](https://dashboard.gocoin.com)<br>
	The GoCoin Dashboard gives you the ability to obtain an API Key<br>
	Click On Developers<br>

##### Navigate to the Developers menu from the dashboard home<br>
![Developers](https://dl.dropboxusercontent.com/s/s4aevk5gig3x0g6/screenshot.png)


More information can be found [here](http://www.gocoin.com/pdfs/merchant_integration_guide_1.0.0.pdf)

### 3. Configuration

1. Install GoCoin Module: Login as Admin and click Modules. From the Modules list locate GoCoin (In the UBERCART - PAYMENT section) and Enable the module by selecting checkbox. Save Configurations<br><br>

2. Configure GoCoin Payment module, Home � Administration � Store � Configuration  � PAYMENT METHOD<br>
  a) Enter Merchant ID<br>
  b) Enter API Key
3. SAVE AGAIN. You are now ready to accept payments with GoCoin!
