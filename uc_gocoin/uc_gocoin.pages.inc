<?php
/**
 * @file 
 *  
 *
 */
file_exists(__dir__ . '/gocoinlib/src/GoCoin.php');
{
    include(__dir__ . '/gocoinlib/src/GoCoin.php');
}
function uc_gocoin_complete($cart_id = 0) {
       sleep(5);     
    watchdog('GoCoin', 'Receiving new order notification for order !order_id.', array('!order_id' => check_plain($_REQUEST['gocoin_order_id'])));
    $order = uc_order_load($_REQUEST['gocoin_order_id']);
          
    if ($order === FALSE || $order->order_status != 'processing') {
        return t('Awaiting Payment Confirmation from GoCoin.');
    }
    else {
        if ( $order->order_status == 'processing' || $order->order_status == 'payment_received') {
            drupal_set_message(t('Your order will be processed as soon as your payment clears at GoCoin.'));
        }
       $_SESSION['uc_checkout'][$_SESSION['cart_order']]['do_complete'] = TRUE;
       drupal_goto('cart/checkout/complete');
    }
}

function uc_gocoin_redirect() {
    $data = uc_order_load(intval($_SESSION['cart_order']));
    $merchant_id = variable_get('uc_gocoin_client_id', '');
    $access_token = variable_get('uc_gocoin_client_secret_key', '');
    $url1 = url('cart/checkout', array('absolute' => TRUE));
    if(!isset($data->order_id)){
        drupal_set_message( 'Order Id not Set', 'error');
        drupal_goto($url1);
        exit;
    }
    if (empty($access_token)) {
        $msg = 'Improper Gateway set up. API Key not found.';
        drupal_set_message($msg, 'error');
        uc_order_comment_save($data->order_id, 0, $msg, 'admin');
        watchdog('GoCoin', $msg,'');
        drupal_goto($url1);
        exit;
    }
    //Check to make sure we have a merchant ID
    elseif (empty($merchant_id)) {
        $msg = 'Improper Gateway set up. Merchant Id not found.';
        drupal_set_message($msg, 'error');
        uc_order_comment_save($data->order_id, 0, $msg, 'admin');
        watchdog('GoCoin', $msg,'');
        drupal_goto($url1);
        exit;
    }
    else{
        
            $customer_name = $data->delivery_first_name . ' ' . $data->delivery_last_name;
            $customer_city = $data->delivery_city;
            $customer_address_1 = $data->delivery_street1;
            $customer_address_2 = $data->delivery_street2;
            $customer_region = uc_get_zone_code($data->delivery_zone);
            $customer_postal_code = $data->delivery_postal_code;
            $customer_country = uc_get_country_data(array('country_id' => $data->delivery_country));
            if ($customer_country === FALSE) {
                $customer_country = array(0 => array('country_iso_code_2' => 'US'));
            }
            $customer_phone = $data->delivery_phone;
            $customer_email = $data->primary_email;
            $options = array(
                "type"                  => 'bill',
                'base_price'            => $data->order_total,
                'base_price_currency'   => variable_get('uc_currency_code', 'USD'),
                'callback_url'          => url('cart/gocoin/callback', array('absolute' => TRUE)),
                'redirect_url'          => url('cart/gocoin/complete', array('absolute' => TRUE)) . '?gocoin_order_id=' . $data->order_id,
                'order_id'              => $data->order_id,
                'customer_name'         => $customer_name,
                'customer_address_1'    => $customer_address_1,
                'customer_address_2'    => $customer_address_2,
                'customer_city'         => $customer_city,
                'customer_region'       => $customer_region,
                'customer_postal_code'  => $customer_postal_code,
                'customer_country'      => $customer_country[0]['country_iso_code_2'],
                'customer_phone'        => $customer_phone,
                'customer_email'        => $customer_email,
            );

            $signature = sign($options, $access_token);
            $options['user_defined_8'] = $signature;

            try {
                $invoice = GoCoin::createInvoice($access_token, $merchant_id, $options);
                 if(isset($invoice->gateway_url)){
                    uc_order_update_status($data->order_id, 'pending');
                    uc_cart_empty();
                    drupal_goto($invoice->gateway_url);
                    exit;
                 }
                 else{
                      $msg='Gocoin Error';
                      drupal_set_message( $msg, 'error');
                      uc_order_comment_save($data->order_id, 0, t($msg), 'admin');
                      drupal_goto($url1);
                      exit;
                 }
                
            } catch (Exception $e) {
                $msg = 'Gocoin Error '.$e->getMessage();
                uc_order_comment_save($data->order_id, 0, t($msg), 'admin');
                watchdog('GoCoin', $msg,'');
                drupal_set_message( $msg, 'error');
                drupal_goto($url1);
                exit;
            }
    }
   
    
}

function uc_gocoin_failed($cart_id = 0) {
    
}

function uc_gocoin_callback() {
    _paymentStandard();
}

            
function postData() {
      //get webhook content
      $response = new stdClass();
      $post_data = file_get_contents("php://input");

      if (!$post_data) {
        $response->error = 'Request body is empty';
      }

      $post_as_json = json_decode($post_data);
      if (is_null($post_as_json)){
        $response->error = 'Request body was not valid json';
      } else {
        $response = $post_as_json;
      }
      return $response;
  }
function _paymentStandard() {
    $sts_canceled             =  'canceled';
    $sts_processing           =  'processing';
    $sts_payment_received     =  'payment_received';
    $key   = variable_get('uc_gocoin_client_secret_key', '');
    
    if(empty($key)){
      watchdog('GoCoin', 'Api Key is  blank', '');
      return false;
    } 
      $data = postData(); 
      
    $module_display = 'gocoin';
    $data = postData();
    if (isset($data->error)){
          watchdog('GoCoin', $data->error, '');           
      }
    else {
        $event_id           = $data -> id;
        $event              = $data -> event;
        $invoice            = $data -> payload;
        $payload_arr        = get_object_vars($invoice) ;
                 ksort($payload_arr);
        $signature          = $invoice -> user_defined_8;
        $sig_comp           = sign($payload_arr, $key);
        $status             = $invoice -> status;
        $order_id           = (int) $invoice -> order_id;
        $order              = uc_order_load((int) $order_id);
        $order_state        = uc_order_status_data($order->order_status, 'state');
        if ($order === FALSE) {
            $msg = "Invalid Order: " . $order_id;
            $msg .= ' Event ID: '. $event_id;
            watchdog('GoCoin', $data->error, '');  
        }
        // Check that if a signature exists, it is valid
        elseif (isset($signature) && ($signature != $sig_comp)) {
          $msg = "Signature : " . $signature . "does not match for Order: " . $order_id ."$sig_comp        |    $signature ";
          $msg .= ' Event ID: '. $event_id;
          uc_order_comment_save($order_id, 0, t($msg), 'admin');
        }
        elseif (empty($signature) || empty($sig_comp) ) {
          $msg = "Signature is blank for Order: " . $order_id;
          $msg .= ' Event ID: '. $event_id;
          uc_order_comment_save($order_id, 0, t($msg), 'admin');
        }
        elseif($signature == $sig_comp) {
            switch($event) {

            case 'invoice_created':
              break;

            case 'invoice_payment_received':
              switch ($status) {
                 case 'ready_to_ship':
                  $msg = 'Order ' . $order_id .' is paid and awaiting payment confirmation on blockchain.';
                  break; 
                case 'paid':
                  $msg = 'Order ' . $order_id .' is paid and awaiting payment confirmation on blockchain.';
                  break;
                case 'underpaid':
                  $msg = 'Order ' . $order_id .' is underpaid.';
                  break;
              }
              $msg .= ' Event ID: '. $event_id;
               uc_order_update_status($order_id, $sts_processing);
               uc_order_comment_save($order_id, 0, t($msg),'order', $sts_processing,true);
              break;

            case 'invoice_merchant_review':
               $msg = 'Order ' . $order_id .' is under review. Action must be taken from the GoCoin Dashboard.';
               $msg .= ' Event ID: '. $event_id; 
               uc_order_update_status($order_id, $sts_processing);
               uc_order_comment_save($order_id, 0, t($msg),'order', $sts_processing,true);
              
              break;

            case 'invoice_ready_to_ship':
              $msg = 'Order ' . $order_id .' has been paid in full and confirmed on the blockchain.';
              $msg .= ' Event ID: '. $event_id;  
              uc_order_update_status($order_id, $sts_payment_received);
              uc_order_comment_save($order_id, 0, t($msg),'order', $sts_payment_received,true);
              
              break;

            case 'invoice_invalid':
              $msg = 'Order ' . $order_id . ' is invalid and will not be confirmed on the blockchain.';
              $msg .= ' Event ID: '. $event_id;  
              uc_order_update_status($order_id, $sts_canceled);
              uc_order_comment_save($order_id, 0, t($msg),'order', $sts_canceled,true);
              break;

            default: 
              $msg = "Unrecognized event type: ". $event;
              $msg .= ' Event ID: '. $event_id;  
              uc_order_comment_save($order_id, 0, t($msg), 'admin',true);
          }
          
            watchdog('GoCoin', $msg, ''); 
        }
    }
                 
}

                    
                    