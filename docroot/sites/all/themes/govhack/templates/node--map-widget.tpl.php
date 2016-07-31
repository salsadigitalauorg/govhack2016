<?php
/**
 * @file
 * Returns the HTML for a node.
 *
 * Complete documentation for this file is available online.
 *
 * @see https://drupal.org/node/1728164
 */
?>
<div id="node-<?php print $node->nid; ?>" class="<?php print $classes; ?> clearfix"<?php print $attributes; ?>>

  <?php print $user_picture; ?>

  <?php print render($title_prefix); ?>
  <?php if (!$page): ?>
    <h2<?php print $title_attributes; ?>><a href="<?php print $node_url; ?>"><?php print $title; ?></a></h2>
  <?php endif; ?>
  <?php if ($is_explore_map): ?>
    <h2>Select a council to view projects</h2>
  <?php else: ?>
    <h2>Projects in this council</h2>
  <?php endif; ?>
  <?php print render($title_suffix); ?>

  <?php if ($display_submitted): ?>
    <div class="submitted">
      <?php print $submitted; ?>
    </div>
  <?php endif; ?>

  <!-- Explore by map widget -->
  <div class="map-widget">
    <div class="map-top-controls"></div>
    <div class="map-and-side-container">
      <div class="map-display-container">
        <div class="active-area"></div>
        <div id="map-display"></div>
      </div>
    </div>
    <div class="map-bottom-controls"></div>
    <div class="loading-screen"><span>Loading...</span></div>
  </div>
  <div class="region-map-full-listing"></div>
  <!-- /Explore by map widget -->

  <div class="content"<?php print $content_attributes; ?>>
    <?php
      // We hide the comments and links now so that we can render them later.
      hide($content['comments']);
      hide($content['links']);
      print render($content);
    ?>
  </div>

  <?php print render($content['links']); ?>

  <?php print render($content['comments']); ?>

  <div class="dpc-pager">
    <?php if (!empty($prev_next_links)): ?>
      <?php if (!empty($prev_next_links['prev'])): ?>
        <div class="dpc-prev"><?php print $prev_next_links['prev']; ?></div>
      <?php endif; ?>
      <?php if (!empty($prev_next_links['next'])): ?>
        <div class="dpc-next"><?php print $prev_next_links['next']; ?></div>
      <?php endif; ?>
    <?php endif; ?>
  </div>

</div>
