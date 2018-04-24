<?php

module_load_include('inc', 'mci_ajax', 'mci_ajax_forms');

/**
 * Basic render link example.
 *
 * INPORTANT: drupal_add_library('system', 'drupal.ajax'); is the key line here!
 *
 * @return string
 */
function mci_ajax_render_link() {
    // drupal_add_library is invoked automatically when a form element has the
    // '#ajax' property, but since we are not rendering a form here, we have to
    // do it ourselves.
    drupal_add_library('system', 'drupal.ajax');
    $intro = t("
    The link below has the <i>use-ajax</i> class applied to it, so if
    javascript is enabled, ajax.js will try to submit it via an AJAX call instead
    of a normal page load. On AJAX call the form/element will be rendered on the page");

    $output = "<div>" . $intro . "</div>";

    // The use-ajax class is special, so that the link will call without causing
    // a page reload. Note the /nojs portion of the path - if javascript is
    // enabled, this part will be stripped from the path before it is called.
    $link = l(t('Click here'), 'ajax_link_callback/nojs/', array('attributes' => array('class' => array('use-ajax'))));
    $output .= "<div id='myDiv'></div><div>$link</div>";
    return $output;
}

/**
 * AJAX-enabled link in a renderable array.
 *
 * Demonstrates a clickable AJAX-enabled link using a renderable array with the
 * #ajax property.
 *
 * A link that is constructed as a renderable array can have the #ajax property,
 * which ensures that the link submission is done without a page refresh. The
 * href of the link is used as the ajax callback, but it degrades gracefully
 * without JavaScript because if the 'nojs' portion of the href is not stripped
 * out by js, the callback will return content as required for a full page
 * reload.
 *
 * The necessary JavaScript file, ajax.js, will be included on the page
 * automatically.
 *
 * @return array
 *   Form API array.
 */
function mci_ajax_render_array_link() {
    $intro = "
    The link below has been rendered as an element with the #ajax property, so if
    javascript is enabled, ajax.js will try to submit it via an AJAX call instead
    of a normal page load. On AJAX call the form/element will be rendered on the page.";
    $build['my_div'] = array(
        '#markup' => $intro . '<div id="myDiv"></div>',
    );
    $build['ajax_link'] = array(
        '#type' => 'link',
        '#title' => t('Click here'),
    // Note the /nojs portion of the href - if javascript is enabled,
    // this part will be stripped from the path before it is called.
        '#href' => 'ajax_link_callback/nojs/',
        '#id' => 'ajax_link',
        '#ajax' => array(
            'wrapper' => 'myDiv',
            'method' => 'html',
        ),
    );
    return $build;
}

/**
 * Callback for link example.
 *
 * Takes different logic paths based on whether Javascript was enabled.
 * If $type == 'ajax', it tells this function that ajax.js has rewritten
 * the URL and thus we are doing an AJAX and can return an array of commands.
 *
 * @param string $type
 *   Either 'ajax' or 'nojs. Type is simply the normal URL argument to this URL.
 *
 * @return string|array
 *   If $type == 'ajax', returns an array of AJAX Commands.
 *   Otherwise, just returns the content, which will end up being a page.
 *
 * @flow $output >> $command >> $page
 */
function mci_ajax_response_callback ($type = 'ajax') {
    if ($type == 'ajax') {
        //@todo: Add form ID or any elemnt that needs to be rendered here!
        $form = drupal_get_form('mci_ajax_multistepform');

        $output = render($form);
        $commands = array();
        $commands[] = _ajax_command_custom_insert('html', '#myDiv', $output);
        $page = array(
            '#type' => 'ajax',
            '#commands' => $commands
        );
        ajax_deliver($page);
    } else {
        $output = t("This is some content delivered via a page load.");
        return $output;
    }
}

/**
 *
 * Helper function that creates a Drupal Ajax 'insert/$method' command.
 *
 * Since drupal has multiple commands, this one replaces ALL of 'insert' type commands.
 *
 * @param $method
 *   A jQuery method string, jQuery method to be used to place the new HTML.
 *      Possible values: 'html', 'replace', 'after', 'before', 'append', 'prepend'.
 * @param $selector
 *   A jQuery selector string. If the command is a response to a request from
 *   an #ajax form element then this value can be NULL.
 * @param $html
 *   The data to use with the jQuery html() method.
 * @param $settings
 *   An optional array of settings that will be used for this command only.
 * @return array
 *
 *
 */
function _ajax_command_custom_insert($method = 'html', $selector, $html, $settings = NULL) {
    return array(
        'command' => 'insert',
        'method' => $method,
        'selector' => $selector,
        'data' => $html,
        'settings' => $settings,
    );
}
