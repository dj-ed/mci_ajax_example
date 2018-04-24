<?php

/**
 * An autocomplete form to look up nodes by title.
 *
 * An autocomplete form which looks up nodes by title in the node table,
 * but must keep track of the nid, because titles are certainly not guaranteed
 * to be unique.
 *
 * @param array $form
 *   Form API form.
 * @param array $form_state
 *   Form API form state.
 *
 *  * @return array
 *   Form array.
 */
function mci_ajax_autocomplete_node_title($form, &$form_state) {

    $form['info'] = array(
        '#markup' => '<div>' . t("This example does a node autocomplete by title. The difference between this and a username autocomplete is that the node title may not be unique, so we have to use the nid for uniqueness, placing it in a parseable location in the textfield.") . '</div>',
    );

    $form['node'] = array(
        '#type' => 'textfield',
        '#title' => t('Choose a node by title'),
        // The autocomplete path is provided in hook_menu in ajax_example.module.
        '#autocomplete_path' => 'autocomplete_node_title_callback',
    );

    $form['actions'] = array(
        '#type' => 'actions',
    );

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
    );

    return $form;
}

/**
 * Node title validation handler.
 *
 * Validate handler to convert our string like "Some node title [3325]" into a
 * nid.
 *
 * In case the user did not actually use the autocomplete or have a valid string
 * there, we'll try to look up a result anyway giving it our best guess.
 *
 * Since the user chose a unique node, we must now use the same one in our
 * submit handler, which means we need to look in the string for the nid.
 *
 * @param array $form
 *   Form API form.
 * @param array $form_state
 *   Form API form state.
 */
function mci_ajax_autocomplete_node_title_validate($form, &$form_state) {
    $title = $form_state['values']['node'];
    $matches = array();

    // This preg_match() looks for the last pattern like [33334] and if found
    // extracts the numeric portion.
    $result = preg_match('/\[([0-9]+)\]$/', $title, $matches);
    if ($result > 0) {
        // If $result is nonzero, we found a match and can use it as the index into
        // $matches.
        $nid = $matches[$result];
        // Verify that it's a valid nid.
        $node = node_load($nid);
        if (empty($node)) {
            form_error($form['node'], t('Sorry, no node with nid %nid can be found', array('%nid' => $nid)));
            return;
        }
    }
    // BUT: Not everybody will have javascript turned on, or they might hit ESC
    // and not use the autocomplete values offered. In that case, we can attempt
    // to come up with a useful value. This is not absolutely necessary, and we
    // *could* just emit a form_error() as below.
    else {
        $nid = db_select('node')
            ->fields('node', array('nid'))
            ->condition('title', db_like($title) . '%', 'LIKE')
            ->range(0, 1)
            ->execute()
            ->fetchField();
    }

    // Now, if we somehow found a nid, assign it to the node. If we failed, emit
    // an error.
    if (!empty($nid)) {
        $form_state['values']['node'] = $nid;
    }
    else {
        form_error($form['node'], t('Sorry, no node starting with %title can be found', array('%title' => $title)));
    }
}

/**
 *
 *
 * @param $form
 * @param $form_state
 */
function mci_ajax_autocomplete_node_title_submit($form, &$form_state) {

    $node = node_load($form_state['values']['node']);
// @TODO Transfer this into another form submit.
//    $view = node_view($node, 'teaser');
//    $output = drupal_render($view);

    drupal_set_message(t('You found node %nid with title %title', array('%nid' => $node->nid, '%title' => $node->title)));
}

/**
 *
 * @param string $string
 */
function mci_ajax_autocomplete_node_title_callback($string = "") {
    $matches = array();
    if ($string) {
        $result = db_select('node')
            ->fields('node', array('nid', 'title'))
            ->condition('title', db_like($string) . '%', 'LIKE')
            ->range(0, 10)
            ->execute();
        foreach ($result as $node) {
            $matches[$node->title . " [$node->nid]"] = check_plain($node->title);
        }
    }

    drupal_json_output($matches);
}


///////////////////////////////////////////
/**
 * Form manipulation through AJAX.
 *
 * AJAX-enabled select element causes replacement of a set of checkboxes
 * based on the selection.
 */
function mci_ajax_autocheckboxes($form, &$form_state) {
    // Since the form builder is called after every AJAX request, we rebuild
    // the form based on $form_state.
    $num_checkboxes = !empty($form_state['values']['howmany_select']) ? $form_state['values']['howmany_select'] : 1;

    $form['howmany_select'] = array(
        '#title' => t('How many checkboxes do you want?'),
        '#type' => 'select',
        '#options' => array(1 => 1, 2 => 2, 3 => 3, 4 => 4),
        '#default_value' => $num_checkboxes,
        '#ajax' => array(
            'callback' => 'mci_ajax_autocheckboxes_callback',
            'wrapper' => 'checkboxes-div',
            // 'method' defaults to replaceWith, but valid values also include
            // append, prepend, before and after.
             'method' => 'html',
            // 'effect' defaults to none. Other valid values are 'fade' and 'slide'.
            'effect' => 'fade',
            // 'speed' defaults to 'slow'. You can also use 'fast'
            // or a number of milliseconds for the animation to last.
             'speed' => 'fast',
            // Don't show any throbber...
            'progress' => array('type' => 'throbber'),
        ),
    );

    $form['checkboxes_fieldset'] = array(
        '#title' => t("Generated Checkboxes"),
        // The prefix/suffix provide the div that we're replacing, named by
        // #ajax['wrapper'] above.
        '#prefix' => '<div id="checkboxes-div">',
        '#suffix' => '</div>',
        '#type' => 'fieldset',
        '#description' => t('This is where we get automatically generated checkboxes'),
    );

    for ($i = 1; $i <= $num_checkboxes; $i++) {
        $form['checkboxes_fieldset']["checkbox_$i"] = array(
            '#type' => 'checkbox',
            '#title' => "Checkbox $i",
        );
    }

    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Submit'),
    );

    return $form;
}

/**
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function mci_ajax_autocheckboxes_callback($form, $form_state) {
    return $form['checkboxes_fieldset'];
}


/** MULTISTEPFORM*/

/**
 * Multistep form definition.
 *
 * @param $form
 * @param $form_state
 * @param bool $no_js
 * @return mixed
 */
function mci_ajax_multistepform($form, &$form_state, $no_js = FALSE) {

    // Provide a wrapper around the entire form, since we'll replace the whole
    // thing with each submit.
    $form['#prefix'] = '<div id="multistep-form-wrapper">';
    $form['#suffix'] = '</div>';
    // We want to deal with hierarchical form values.
    $form['#tree'] = TRUE;
    $form['description'] = array(
        '#markup' => '<div>' . t('AJAX Multistep-form example ',
                array('!ajax' => url('examples/ajax_example/wizard'), '!multistep' => url('examples/ajax_example/wizard_no_js')))
            . '</div>',
    );

    // $form_state['storage'] has no specific drupal meaning, but it is
    // traditional to keep variables for multistep forms there.
    $step = empty($form_state['storage']['step']) ? 1 : $form_state['storage']['step'];
    $form_state['storage']['step'] = $step;

    switch ($step) {
        case 1:
            $form['step1'] = array(
                '#type' => 'fieldset',
                '#title' => t('Step 1: Personal details'),
            );
            $form['step1']['name'] = array(
                '#type' => 'textfield',
                '#title' => t('Your name'),
                '#default_value' => empty($form_state['values']['step1']['name']) ? '' : $form_state['values']['step1']['name'],
                '#required' => TRUE,
            );
            break;

        case 2:
            $form['step2'] = array(
                '#type' => 'fieldset',
                '#title' => t('Step 2: Street address'),
            );
            $form['step2']['address'] = array(
                '#type' => 'textfield',
                '#title' => t('Your street address'),
                '#default_value' => empty($form_state['values']['step2']['address']) ? '' : $form_state['values']['step2']['address'],
                '#required' => TRUE,
            );
            break;

        case 3:
            $form['step3'] = array(
                '#type' => 'fieldset',
                '#title' => t('Step 3: City'),
            );
            $form['step3']['city'] = array(
                '#type' => 'textfield',
                '#title' => t('Your city'),
                '#default_value' => empty($form_state['values']['step3']['city']) ? '' : $form_state['values']['step3']['city'],
                '#required' => TRUE,
            );
            break;
    }
    if ($step == 3) {
        $form['submit'] = array(
            '#type' => 'submit',
            '#value' => t("Submit"),
        );
    }
    if ($step < 3) {
        $form['next'] = array(
            '#type' => 'submit',
            '#value' => t('Next step'),
            '#ajax' => array(
                'wrapper' => 'multistep-form-wrapper',
                'callback' => 'mci_ajax_multistepform_callback',
            ),
        );
    }
    if ($step > 1) {
        $form['prev'] = array(
            '#type' => 'submit',
            '#value' => t("Previous step"),

            // IMPORTANT: Since all info will be discarded, don't validate on 'prev'.
            '#limit_validation_errors' => array(),
            // #submit is required to use #limit_validation_errors
            '#submit' => array('mci_ajax_multistepform_submit'),
            '#ajax' => array(
                'wrapper' => 'multistep-form-wrapper',
                'callback' => 'mci_ajax_multistepform_callback',
            ),
        );
    }

    // Unsets #ajax from prev/next buttons if no ajax is loaded.
    // Form will be rebuild on page reload.
    if ($no_js) {
        unset($form['next']['#ajax']);
        unset($form['prev']['#ajax']);
    }

    return $form;
}

/**
 * Multistepform callback - its sole purpose is to rebuild form when navigate/submit through steps.
 *
 * @param $form
 * @param $form_state
 * @return $form['rebuilt']
 */
function mci_ajax_multistepform_callback($form, $form_state) {
    return $form;
}

/**
 * Multistep form submit function.
 *
 * @param $form
 * @param $form_state
 * @return
 */
function mci_ajax_multistepform_submit($form, &$form_state) {
    $form_state['rebuild'] = TRUE;

    // Save away the current information.
    $current_step = $form_state['storage']['step'];
    if (!empty($form_state['values'][$current_step])) {
        $form_state['storage']['values'][$current_step] = $form_state['values'][$current_step];
    }

    switch ($form_state['triggering_element']['#value']) {
        case t('Next step'):
            $form_state['storage']['step']++;
            // If values have already been entered for this step, recover them from
            // $form_state['storage'] to pre-populate them.
            $step_name = 'step' . $form_state['storage']['step'];
            if (!empty($form_state['storage']['values'][$step_name])) {
                $form_state['values'][$step_name] = $form_state['storage']['values'][$step_name];
            }
            break;
        case t('Previous step'):
            $form_state['storage']['step']--;
            // Recover our values from $form_state['storage'] to pre-populate them.
            $step_name = 'step' . $form_state['storage']['step'];
            $form_state['values'][$step_name] = $form_state['storage']['values'][$step_name];
            break;
        case t('Submit your information'):
            $value_message = t('Your information has been submitted:') . ' ';
            foreach ($form_state['storage']['values'] as $step => $values) {
                $value_message .= "$step: ";
                foreach ($values as $key => $value) {
                    $value_message .= "$key=$value, ";
                }
            }

            drupal_set_message($value_message);
            $form_state['rebuild'] = FALSE;
            break;
    }

    drupal_set_message($current_step);

    return FALSE;
}


