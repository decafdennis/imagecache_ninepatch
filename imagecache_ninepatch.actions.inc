<?php
// Developed by Dennis Stevense, naquah.net

function imagecache_ninepatch_image_form($action) {
  imagecache_ninepatch_toolkit_warn();
  
  $form = array();
  
  $form['foreground'] = array(
    '#type' => 'radios',
    '#title' => t('Composition'),
    '#default_value' => isset($action['foreground']) ? $action['foreground'] : 0,
    '#options' => array(
      0 => t('Background'),
      1 => t('Overlay'),
    ),
  );
  
  $form['top'] = array(
    '#type' => 'textfield',
    '#title' => t('Top border height'),
    '#default_value' => isset($action['top']) ? $action['top'] : 0,
    '#size' => 6,
    '#description' => t('The amount of pixels at the top of the image that should not be scaled.'),
    '#required' => TRUE,
    '#element_validate' => array('imagecache_ninepatch_validate_number'),
  );
  $form['right'] = array(
    '#type' => 'textfield',
    '#title' => t('Right border width'),
    '#default_value' => isset($action['right']) ? $action['right'] : 0,
    '#size' => 6,
    '#description' => t('The amount of pixels at the right side of the image that should not be scaled.'),
    '#required' => TRUE,
    '#element_validate' => array('imagecache_ninepatch_validate_number'),
  );
  $form['bottom'] = array(
    '#type' => 'textfield',
    '#title' => t('Bottom border height'),
    '#default_value' => isset($action['bottom']) ? $action['bottom'] : 0,
    '#size' => 6,
    '#description' => t('The amount of pixels at the bottom of the image that should not be scaled.'),
    '#required' => TRUE,
    '#element_validate' => array('imagecache_ninepatch_validate_number'),
  );
  $form['left'] = array(
    '#type' => 'textfield',
    '#title' => t('Left border height'),
    '#default_value' => isset($action['left']) ? $action['left'] : 0,
    '#size' => 6,
    '#description' => t('The amount of pixels at the left side of the image that should not be scaled.'),
    '#required' => TRUE,
    '#element_validate' => array('imagecache_ninepatch_validate_number'),
  );
  
  $form['image_path'] = array(
    '#type' => 'textfield',
    '#title' => t('9-patch image file path'),
    '#default_value' => isset($action['image_path']) ? $action['image_path'] : '',
    '#description' => t('The path of the 9-patch image file relative to the Drupal files directory.'),
    '#required' => TRUE,
    '#element_validate' => array('imagecache_ninepatch_validate_file'),
  );
  
  return $form;
}

function imagecache_ninepatch_image_image(&$image, $action = array()) {
  if (!imagecache_ninepatch_toolkit_check($image->toolkit)) {
    return;
  }
  
  $path = file_create_path($action['image_path']);
  $ninepatch = imageapi_image_open($path, 'imageapi_gd');

  $src_width = $ninepatch->info['width'];
  $src_height = $ninepatch->info['height'];
  $src_x = array(
    0,
    $action['left'],
    $src_width - $action['right'],
  );
  $src_y = array(
    0,
    $action['top'],
    $src_height - $action['bottom'],
  );
  $src_w = array(
    $action['left'],
    $src_width - $action['left'] - $action['right'],
    $action['right'],
  );
  $src_h = array(
    $action['top'],
    $src_height - $action['top'] - $action['bottom'],
    $action['bottom'],
  );
  
  $dst_width = $image->info['width'];
  $dst_height = $image->info['height'];
  $dst_x = array(
    0,
    $action['left'],
    $dst_width - $action['right'],
  );
  $dst_y = array(
    0,
    $action['top'],
    $dst_height - $action['bottom'],
  );
  $dst_w = array(
    $action['left'],
    $dst_width - $action['left'] - $action['right'],
    $action['right'],
  );
  $dst_h = array(
    $action['top'],
    $dst_height - $action['top'] - $action['bottom'],
    $action['bottom'],
  );

  $ninepatch_layer = imagecreatetruecolor($dst_width, $dst_height);

  for ($x = 0; $x < 3; $x++) {
    for ($y = 0; $y < 3; $y++) {
      if ($src_w[$x] == $dst_w[$x] && $src_h[$y] == $dst_h[$y])
        imagecopy($ninepatch_layer, $ninepatch->resource, $dst_x[$x], $dst_y[$y], $src_x[$x], $src_y[$y], $src_w[$x], $src_h[$y]);
      else
        imagecopyresampled($ninepatch_layer, $ninepatch->resource, $dst_x[$x], $dst_y[$y], $src_x[$x], $src_y[$y], $dst_w[$x], $dst_h[$y], $src_w[$x], $src_h[$y]);
    }
  }

  imageapi_image_close($ninepatch);
  
  imagealphablending($ninepatch_layer, TRUE);
  imagealphablending($image->resource, TRUE);
  if ($action['foreground']) {
    imagecopy($image->resource, $ninepatch_layer, 0, 0, 0, 0, $dst_width, $dst_height);
    imagedestroy($ninepatch_layer);
  }
  else {
    imagecopy($ninepatch_layer, $image->resource, 0, 0, 0, 0, $dst_width, $dst_height);
    imagedestroy($image->resource);
    $image->resource = $ninepatch_layer;
  }
  
  return TRUE;
}

function imagecache_ninepatch_toolkit_check($toolkit = NULL) {
  if ($toolkit == NULL) {
    $toolkit = imageapi_default_toolkit();
  }
  return $toolkit == 'imageapi_gd';
}

function imagecache_ninepatch_toolkit_warn() {
  if (!imagecache_ninepatch_toolkit_check()) {
    drupal_set_message(t('The 9-patch actions support the GD image toolkit only.'), 'warning');
  }
}

function imagecache_ninepatch_validate_number($element) {
  if (!empty($element['#value']) && !is_numeric($element['#value'])) {
    form_error($element, t('This is not a valid number.'));
  }
}

function imagecache_ninepatch_validate_file($element) {
  if (!empty($element['#value']) && !file_exists(file_create_path($element['#value']))) {
    form_error($element, t('No file exists at the given path.'));
  }
}
