<?php

/**
 * @file
 *  
 *
 */
function uc_gocoin_complete($cart_id = 0) {
            
    watchdog('Gocoin', 'Receiving new order notification for order !order_id.', array('!order_id' => check_plain($_REQUEST['gocoin_order_id'])));
    $order = uc_order_load($_REQUEST['gocoin_order_id']);
          
    if ($order === FALSE || $order->order_status != 'processing') {
        return t('An error has occurred during payment.  Please contact us to ensure your order has submitted.');
    }
    else {
        if ( $order->order_status == 'processing') {
            drupal_set_message(t('Your order will be processed as soon as your payment clears at Gocoin.'));
            uc_order_comment_save($order->order_id, 0, t('Payment process approval at Gocoin.'), 'admin');
        }
       $_SESSION['uc_checkout'][$_SESSION['cart_order']]['do_complete'] = TRUE;
       drupal_goto('cart/checkout/complete');
    }
}

function uc_gocoin_redirect() {

    $url1 = isset($_POST['gocoin_url']) && !empty($_POST['gocoin_url']) ? $_POST['gocoin_url'] : '';
    if ($url1 != '') {
        // echo $url1;
        drupal_goto($url1);
        exit;
    } else {
        $url1 = url('cart/gocoin/checkout', array('absolute' => TRUE));
        drupal_goto($url1);
        exit;
    }
}

function uc_gocoin_failed($cart_id = 0) {
    
}

function uc_gocoin_callback() {
    _paymentStandard();
}

function uc_gocoin_create_token() {
    if (isset($_REQUEST['code'])) {
        $code = $_REQUEST['code'];
    } else {
        $code = '';
    }


    $client_id = variable_get('uc_gocoin_client_id', '');
    $client_secret = variable_get('uc_gocoin_client_secret_key', '');
    try {
        $token = GoCoin::requestAccessToken($client_id, $client_secret, $code, null);
        echo "<b>Copy this Access Token into your GoCoin Module: </b><br>" . $client->getToken();
    } catch (Exception $e) {
        echo "Problem in getting Token: " . $e->getMessage();
    }
    die();
}

function getNotifyData() {
    $post_data = file_get_contents("php://input");
    if (!$post_data) {
        $response = new stdClass();
        $response->error = 'Post Data Error';
        return $response;
    }
    $response = json_decode($post_data);
    return $response;
}

function _paymentStandard() {

    $module_display = 'gocoin';
    $response = getNotifyData();
    if (!$response) {
        //======================Error=============================     
    }
    if (isset($response->error) && $response->error != '') {
        
    }
    if (isset($response->payload)) {
        //======================IF Response Get=============================     
        $err = FALSE;
        $description = '';

        $event = $response->event;
        $order_id = (int) $response->payload->order_id;
        $redirect_url = $response->payload->redirect_url;
        $transction_id = $response->payload->id;
        $total = $response->payload->base_price;
        $status = $response->payload->status;
        $currency_id = $response->payload->user_defined_1;
        $secure_key = $response->payload->user_defined_2;
        $currency = $response->payload->base_price_currency;
        $currency_type = $response->payload->price_currency;
        $invoice_time = $response->payload->created_at;
        $expiration_time = $response->payload->expires_at;
        $updated_time = $response->payload->updated_at;
        $merchant_id = $response->payload->merchant_id;
        $btc_price = $response->payload->price;
        $price = $response->payload->base_price;
        $url = "https://gateway.gocoin.com/merchant/" . $merchant_id . "/invoices/" . $transction_id;

        //=================== Set To Array=====================================//
        //Used for adding in db

        $iArray = array(
            'order_id' => $order_id,
            'invoice_id' => $transction_id,
            'url' => $url,
            'status' => $event,
            'btc_price' => $btc_price,
            'price' => $price,
            'currency' => $currency,
            'currency_type' => $currency_type,
            'invoice_time' => $invoice_time,
            'expiration_time' => $expiration_time,
            'updated_time' => $updated_time);
        addTransaction($type = 'payment', $iArray);
        $sts = '';
        $msg = '';
        switch ($status) {
            case 'paid':
                $sts = 'processing';
                $msg = 'A payment has been accepted.';
                break;

            case 'unpaid':
                $sts = ''; // Failed
                $msg = 'A payment has been declined.';
                break;

            default:
                $sts = ''; // Failed
                $msg = ' A payment Error:';
                break;
        }

        if ($event == 'invoice_created') {
            $order = uc_order_load((int) $order_id);
            $order_state = uc_order_status_data($order->order_status, 'state');
            if ($order_state != 'in_checkout' && $order_state != 'post_checkout') {
                $err = TRUE;
                $description = 'Declined. The order is paid, complete or canceled.';
            }

            if ($order === FALSE) {
                
            } else {
                if ($sts != '') {
                    uc_order_update_status($order_id, $sts);
                }
                uc_order_comment_save($order_id, 0, t($msg), 'admin');
            }
        }
    }
}
// function to save in ipn
function addTransaction($type = 'payment', $details) {
    return $query = db_query("
      INSERT INTO uc_payment_gocoin_ipn (  order_id, invoice_id, url, status, btc_price,
      price, currency, currency_type, invoice_time, expiration_time, updated_time)
      VALUES ( 
          '" . $details['order_id'] . "',
          '" . $details['invoice_id'] . "',
          '" . $details['url'] . "',
          '" . $details['status'] . "',
          '" . $details['btc_price'] . "',
          '" . $details['price'] . "',
          '" . $details['currency'] . "',
          '" . $details['currency_type'] . "',
          '" . $details['invoice_time'] . "',
          '" . $details['expiration_time'] . "',
          '" . $details['updated_time'] . "' )");
}
