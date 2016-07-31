var BREAKPOINT_MOBILE = 480;
var BREAKPOINT_TABLET = 768;
var BREAKPOINT_DESKTOP = 992;
var BREAKPOINT_DESKTOP_LARGE = 1200;

/**
 * Regional Map.
 */
function RegionalMap() {

  var $ = jQuery;

  var default_map_settings = {
    center: [-37.829920943955045, 144.93988037109375],
    zoom: 7,
    zoomControl: false,
    scrollWheelZoom: false
  };

  var markers = L.markerClusterGroup({
    disableClusteringAtZoom: 15
  });

  var victorian_geodata_bounds = L.latLngBounds(L.latLng(-39.13658096, 140.96190245000003), L.latLng(-33.98128284999996, 149.97629085000005));

  var is_ie9 = test_ie9();

  // Map Variables.
  var map = null;
  var tile_layer = null;
  var geodata_cache = [];
  var markers_map = {};
  var geojson_group = L.geoJson();
  var use_geojson_files_from_theme = false;

  var has_map = true;
  var map_focus_animate = true;

  var initial_filter_region_nid = null;
  var initial_filter_priority_id = null;
  var initial_filter_ipd_nid = null;

  var base_path = document.location.protocol + '//' + document.location.host;
  var theme_path = '/sites/all/themes/custom/budget_theme/';

  var locations_url = base_path + '/api/views/locations?args[0]=3000';
  var employment_agencies_url = base_path + '/api/views/employment_agencies?args[0]=3000';
  var employment_statistics_url = base_path + '/api/views/employment_statistics?args[0]=3000';
  var location_data_url = base_path + '/api/views/location_data?args[0]=3000';

  var split_geo_url = theme_path + '/vendor/map/region_geodata';
  var vic_geo_url = theme_path + '/vendor/map/vic_geodata.json';

  var locations_data = null;
  var priority_data = null;
  var region_data = null;
  var employment_statistics = null;

  // Data Constants.
  var EMPLOYMENT_STAT_NID = 'employment_stat_nid';
  var EMPLOYMENT_STAT_TITLE = 'employment_stat_title';
  var EMPLOYMENT_STAT_LOCATION_TITLE = 'location_title';
  var EMPLOYMENT_STAT_AVG_SALARY = 'employment_stat_average_salary';
  var EMPLOYMENT_STAT_BODY = 'employment_stat_body';
  var EMPLOYMENT_STAT_DATE = 'employment_stat_date';
  var EMPLOYMENT_STAT_FEM_SALARY = 'employment_stat_female_salary';
  var EMPLOYMENT_STAT_FEM_TOL_EMP = 'employment_stat_female_total_employment';
  var EMPLOYMENT_STAT_MALE_SALARY = 'employment_stat_male_salary';
  var EMPLOYMENT_STAT_MALE_TOT_EMP = 'employment_stat_male_total_employment';
  var EMPLOYMENT_STAT_TOT_EMP = 'employment_stat_total_employment';
  var EMPLOYMENT_STAT_LOC_ID = 'location_nid';
  var EMPLOYMENT_STAT_INDUSTRY_NAME = 'name';
  var EMPLOYMENT_STAT_INDUSTRY_TID = 'industry_tid';
  var EMPLOYMENT_STAT_INDUSTRY_BODY = 'employ_industry_body';
  var EMPLOYMENT_STAT_INDUSTRY_NUM_JOB = 'employ_industry_number_jobs';
  var EMPLOYMENT_STAT_INDUSTRY_PROJ_NUM_JOB = 'employ_industry_projected_num_jobs';

// location
// sa2_code
// total_persons
// name
// body
// location_code
// location_latitude
// locaion_longitude
// location_shapefile
// state
// location_nid
// sa2_tid
  var REGION_NID = 'location_nid';
  var REGION_TITLE = 'location';
  var REGION_LATITUDE = 'location_latitude';
  var REGION_LONGITUDE = 'locaion_longitude';
  var REGION_LGA_CODE = 'location_code';
  var REGION_LOCATION = 'state';
  var REGION_GEOJSON = 'geojson';

  var PRIORITY_COLOUR = 'colour';
  var PRIORITY_TITLE = 'title';
  var PRIORITY_NID = 'nid';

  var SEARCH_PROJCT_PLACEHOLDER = 'Search projects...';

  // Styles
  var style_region = {
    color: '#D50032',
    weight: 1,
    opacity: 0.75,
    fillOpacity: 0.25,
    fillColor: '#D50032'
  };

  // Popup
  var is_popup_open = false;
  var popup_showing_id = -1;

  // =========================================================
  // UTIL METHODS
  // =========================================================
  function get_employment_statistics_item(id) {
    for (var i = 0; i < employment_statistics.length; i++) {
      if (employment_statistics[i][EMPLOYMENT_STAT_NID] == id) {
        return employment_statistics[i];
      }
    }
    return null;
  }

  function trim_trailing_zeroes(value) {
    return '$' + eval(value.replace('$', ''));
  }

  function get_priority_data_item(id) {
    for (var i = 0; i < priority_data.length; i++) {
      if (priority_data[i][PRIORITY_NID] == id) {
        return priority_data[i];
      }
    }
    return null;
  }

  function get_region_data_item_from_code(lga_code) {
    for (var i = 0; i < region_data.length; i++) {
      if (region_data[i][REGION_LGA_CODE] == lga_code) {
        return region_data[i];
      }
    }
    return null;
  }

  function get_region_data_item(id) {
    for (var i = 0; i < region_data.length; i++) {
      if (region_data[i][REGION_NID] == id) {
        return region_data[i];
      }
    }
    return null;
  }

  function load_geodata_layer(lga_code, callback) {
    if (geodata_cache[lga_code] === undefined) {
      var region_url = null;
      // A "vic" lga_code will return a special victorian region.
      if (lga_code === "vic") {
        region_url = vic_geo_url;
      }
      else {
        var region = get_region_data_item_from_code(lga_code);
        region_url = (region !== null) ? region[REGION_GEOJSON] : null;
        if (use_geojson_files_from_theme) {
          region_url = split_geo_url + '/lga_geodata_' + lga_code + '.json';
        }
      }
      if (region_url !== null) {
        $.ajax({
          url: region_url,
          success: function(data) {
            var geojson = L.geoJson(data, {
              style: style_region
            });
            geodata_cache[lga_code] = geojson;
            if (callback !== undefined) {
              callback(geodata_cache[lga_code]);
            }
          }
        });
      }
      else {
        if (callback !== undefined) {
          callback(null);
        }
      }
    }
    else {
      if (callback !== undefined) {
        callback(geodata_cache[lga_code]);
      }
    }
  }

  function get_priority_color(employment_statistics_item) {
    var priority = get_priority_data_item(employment_statistics_item[EMPLOYMENT_STAT_LOCATION_TITLE]);
    return (priority === null || priority[PRIORITY_COLOUR] === null) ? 'dpc_priority_0' : priority[PRIORITY_COLOUR];
  }

  function get_marker(number, employment_statistics_item) {
    var rtn = L.ExtraMarkers.icon({
      icon: 'fa-number',
      markerColor: get_priority_color(employment_statistics_item),
      number: idOf(number)
    });
    return rtn;
  }

  function idOf(i) {
    return (i >= 26 ? idOf((i / 26 >> 0) - 1) : '') + 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' [i % 26 >> 0];
  }

  function point_in_polygon(point, vs) {
    // https://github.com/substack/point-in-polygon/blob/master/index.js
    // ray-casting algorithm based on
    // http://www.ecse.rpi.edu/Homepages/wrf/Research/Short_Notes/pnpoly.html
    var x = point[0],
      y = point[1];
    var inside = false;
    for (var i = 0, j = vs.length - 1; i < vs.length; j = i++) {
      var xi = vs[i][0],
        yi = vs[i][1];
      var xj = vs[j][0],
        yj = vs[j][1];
      var intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
      if (intersect) inside = !inside;
    }
    return inside;
  }

  function test_ie9() {
    var rtn = false;
    var myNav = navigator.userAgent.toLowerCase();
    if (myNav.indexOf('msie') != -1) {
      var ie_ver = parseInt(myNav.split('msie')[1]);
      if (ie_ver <= 9) {
        rtn = true;
      }
    }
    return rtn;
  }

  function get_url_query_values() {
    var key = document.location.pathname + '?';
    var query_start = document.location.href.indexOf(key);
    if (query_start >= 0) {
      var str = document.location.href.substr(query_start + key.length);
      var str_arr = str.split('&');
      var rtn = {};
      for (var i = 0; i < str_arr.length; i++) {
        var key_val = str_arr[i].split('=');
        rtn[key_val[0]] = key_val[1];
      }
      return rtn;
    }
    else {
      return [];
    }
  }

  function set_restriction_bounds() {
    // Set restriction bounds.
    var bounds = victorian_geodata_bounds;
    // Region map needs a buffer on the victorian bounds.
    var threshold = 8;
    var sw = L.latLng(bounds._southWest.lat - threshold, bounds._southWest.lng - threshold);
    var ne = L.latLng(bounds._northEast.lat + threshold, bounds._northEast.lng + threshold);
    bounds = L.latLngBounds(sw, ne);
    // Lock depth and bounds that can be seen.
    map.setMaxBounds(bounds);
    map.options.minZoom = 6;
  }

  // =========================================================
  // MAP
  // =========================================================
  function initialize_map() {
    $('.map-widget').addClass('has-map');
    map = L.map('map-display', default_map_settings).setActiveArea('active-area');

    $('.map-widget').addClass('regional-map');
    tile_layer = L.tileLayer('//{s}.tile.openstreetmap.se/hydda/full/{z}/{x}/{y}.png', {
      attribution: 'Tiles courtesy of <a href="http://openstreetmap.se/" target="_blank">OpenStreetMap Sweden</a> &mdash; Map data &copy; <a href="http://www.openstreetmap.org/copyright">OpenStreetMap</a>'
    });
    map.addLayer(tile_layer);
    console.log(tile_layer);
    console.log(map);
    return true;
  }

  function load_all_data(callback) {
    var completed_calls = 0;

    function data_load_complete(data) {
      completed_calls++;
      if (completed_calls === 3) {
        if (callback !== undefined) {
          callback();
        }
      }
    }
    $.ajax({
      url: employment_statistics_url,
      success: function(rtn_employment_statistics) {
        // Remove any blank NIDS from the data.
        var parsed_data = [];
        var data_len = rtn_employment_statistics.length;
        for (var i = 0; i < data_len; i++) {
          if (rtn_employment_statistics[i]['location_nid'].length > 0) {
            parsed_data.push(rtn_employment_statistics[i]);
          }
        }
        employment_statistics = parsed_data;
        data_load_complete();
      }
    });
    // $.ajax({
    //   url: location_data_url,
    //   success: function(rtn_priority_data) {
    //     priority_data = rtn_priority_data;
    //     data_load_complete();
    //   }
    // });
    load_region_data(data_load_complete);
  }

  function load_region_data(callback) {
    $.ajax({
      url: locations_url,
      success: function(rtn_region_data) {
        region_data = rtn_region_data;
        if (callback !== undefined) {
          callback();
        }
      }
    });
  }

  function parse_employment_statistics() {
    // Add Markers.
    var marker_priority_count = {};
    for (var i = 0; i < employment_statistics.length; i++) {
      var id = employment_statistics[i][EMPLOYMENT_STAT_NID];
      var title = employment_statistics[i][EMPLOYMENT_STAT_TITLE];
      var lat = employment_statistics[i][EMPLOYMENT_STAT_DATE];
      var lng = employment_statistics[i][EMPLOYMENT_STAT_FEM_SALARY];
      var priority = employment_statistics[i][EMPLOYMENT_STAT_LOCATION_TITLE];
      // priority = (priority !== null && priority.length > 0) ? priority : 'none';

      if (lat !== null && lng !== null && lat !== "N/a" && lng !== "N/a") {
        if (marker_priority_count[priority] === undefined) {
          marker_priority_count[priority] = 0;
        }
        var marker = L.marker([lat, lng], {
          icon: get_marker(marker_priority_count[priority], employment_statistics[i])
        });
        marker.bindPopup('<span class="info-text">' + title + '</span>' + '<button class="info-btn" data-id="' + id + '">More info</button>');
        markers.addLayer(marker);
        markers_map[id] = marker;
        employment_statistics[i]['_statewide'] = false;
        marker_priority_count[priority]++;
      }
      else {
        employment_statistics[i]['_statewide'] = true;
      }
    }
    region_first_render();
  }

  function region_first_render() {
    $('.loading-screen').addClass('hidden');

    // Render on map.
    if (has_map) {
      geojson_group.addTo(map);
      set_restriction_bounds();
    }

    region_add_listeners();
    populate_filters();

    if (has_map) {
      map.invalidateSize(false);
    }

    // Populate filters with any presets.
    if (initial_filter_region_nid !== null) {
      var region_item = get_region_data_item(initial_filter_region_nid);
      if (region_item !== null) {
        $('#region-filter').val(region_item[REGION_NID]);
      }
    }
    if (initial_filter_priority_id !== null) {
      $('#priorities-filter').val(initial_filter_priority_id);
    }

    map_focus_animate = false;
    var ipd_exists = (initial_filter_ipd_nid !== null);

    // Don't focus map if IPD exists.
    region_filter(!ipd_exists);

    if (has_map) {
      map.addLayer(markers);
    }

    if (ipd_exists) {
      open_popup(get_employment_statistics_item(initial_filter_ipd_nid));
    }
    map_focus_animate = true;
  }

  function region_initialize_controls() {

    // Top.
    var html = '';
    html += '<span class="filter-option"><span class="select-box"><select id="region-filter"></select></span></span>';
    html += '<span class="filter-option"><span class="select-box"><select id="priorities-filter"></select></span></span>';
    html += '<span class="filter-option"><input id="search-filter" type="text" placeholder="' + SEARCH_PROJCT_PLACEHOLDER + '" /></span>';
    html += '<span class="statewide-filter"><input id="statewide-projects-top" type="checkbox" /><label for="statewide-projects-top">Hide statewide projects</label></span>';
    $('.map-top-controls').html(html);

    // Sidebar.
    html = '';
    html += '<div id="map-sidebar">';
    html += '<div class="popup">';
    html += '<button class="popup-close">close</button>';
    html += '<div class="popup-content"></div>';
    html += '</div>';
    html += '<div class="sidebar-content"></div>';
    html += '</div>';
    $('.map-and-side-container').append(html);

    // Bottom.
    html = '';
    if (has_map) {
      html += '<span class="zoom-controls">';
      html += '<button id="reset-zoom">Reset to council</button>';
      html += '<button id="zoom-out" aria-label="Zoom Out">-</button>';
      html += '<button id="zoom-in" aria-label="Zoom In">+</button>';
      html += '<label for="zoom-in">Zoom</label>';
      html += '</span>';
      html += '<span class="statewide-filter"><input id="statewide-projects-bottom" type="checkbox" /><label for="statewide-projects-bottom">Hide statewide projects</label></span>';
    }
    $('.map-bottom-controls').html(html);
  }

  // =========================================================
  // REGION MAP - POPULATE
  // =========================================================
  function populate_filters() {
    // Populate region filter.
    region_data.sort(function(a, b) {
      return (a[REGION_TITLE] > b[REGION_TITLE]) ? 1 : -1;
    });
    var html = '<option value="">Victoria</option>';
    for (var l = 0; l < region_data.length; l++) {
      var area = region_data[l];
      html += '<option value="' + area[REGION_NID] + '">' + area[REGION_TITLE] + '</option>';
    }
    $('#region-filter').html(html);

    // Populate priority filter.
    var sorted_priorities = [];
    sorted_priorities = priority_data.sort(function(a, b) {
      return (a[PRIORITY_TITLE] > b[PRIORITY_TITLE]) ? 1 : -1;
    });
    html = '<option value="">Select priorities</option>';
    for (var k = 0; k < sorted_priorities.length; k++) {
      var priority = sorted_priorities[k];
      html += '<option value="' + priority[PRIORITY_NID] + '">' + priority[PRIORITY_TITLE] + '</option>';
    }
    $('#priorities-filter').html(html);
  }

  // =========================================================
  // REGION MAP - FILTER
  // =========================================================
  function region_change_filter() {
    // Store Filter Data.
    var priorities_val = $('#priorities-filter').val();
    $.cookie('priorities-filter', priorities_val);
    // Hide Popup.
    if (is_popup_open) {
      close_popup();
    }
    // Update Filters.
    region_filter(true);
  }

  function region_filter(focus_map) {
    // Get Values
    var region_val = $('#region-filter').val();
    var priorities_val = $('#priorities-filter').val();
    var search_val = $('#search-filter').val();

    // Copy region data.
    var filtered_results = employment_statistics.slice();

    // Region Filter.
    filtered_results = filter_regions(region_val, filtered_results);

    // Priorities Filter.
    filtered_results = filter_priorities(priorities_val, filtered_results);

    // Search Filter.
    filtered_results = filter_search(search_val, filtered_results);

    // Draw.
    if (has_map) {
      draw_pins(filtered_results);
      show_soft_loading(true);
      region_draw_geodata(region_val, function() {
        if (focus_map) {
          show_soft_loading(false);
          draw_map_focus(region_val);
        }
      });
    }
    draw_page_title(region_val);
    draw_listing(filtered_results);
  }

  // =========================================================
  // FILTER REGIONS / PRIORITY
  // =========================================================
  function filter_regions(word, arr) {
    var filter_show_all = (word === "");
    var include_statewide = !($('#statewide-projects-top').is(':checked'));
    var rtn = [];

    for (var i = 0; i < arr.length; i++) {
      var item_is_statewide = (arr[i]['_statewide'] === true);

      if (item_is_statewide && include_statewide) {
        rtn.push(arr[i]);
      }
      else if (!item_is_statewide) {
        if (filter_show_all) {
          rtn.push(arr[i]);
        }
        else if (arr[i][EMPLOYMENT_STAT_INDUSTRY_NAME] === word) {
          rtn.push(arr[i]);
        }
      }
    }

    return rtn;
  }

  function filter_priorities(word, arr) {
    return filter_on_array(word, arr, EMPLOYMENT_STAT_LOCATION_TITLE);
  }

  function filter_on_array(word, arr, property) {
    if (word === "") {
      return arr;
    }
    else {
      var filtered_listing = [];
      for (var i = 0; i < arr.length; i++) {
        if (arr[i][property] == word) {
          filtered_listing.push(arr[i]);
        }
      }
      return filtered_listing;
    }
  }

  // =========================================================
  // FILTER SEARCH
  // =========================================================
  function filter_search(word, arr) {
    if (word === "" || (is_ie9 && word === SEARCH_PROJCT_PLACEHOLDER)) {
      return arr;
    }
    else {
      var filtered_listing = [];
      var word_upper = word.toUpperCase();
      for (var i = 0; i < arr.length; i++) {
        var search_text = arr[i][EMPLOYMENT_STAT_TITLE].toUpperCase();
        if (search_text.indexOf(word_upper) >= 0) {
          filtered_listing.push(arr[i]);
        }
      }
      return filtered_listing;
    }
  }

  // =========================================================
  // DRAW
  // =========================================================
  function region_draw_geodata(word, callback) {
    var search_for_lga = "";
    // If Victoria filter, LGA set to "vic" will return a special victorian region.
    if (word === "") {
      search_for_lga = "vic";
    }
    else {
      var region_item = get_region_data_item(word);
      search_for_lga = region_item[REGION_LGA_CODE];
    }
    // Load Geodata and display.
    load_geodata_layer(search_for_lga, function(geo_layer) {
      geojson_group.clearLayers();
      if (geo_layer !== null) {
        geojson_group.addLayer(geo_layer);
      }
      if (callback !== undefined) {
        callback();
      }
    });
  }

  function draw_pins(list) {
    markers.clearLayers();
    // Only draw relevant pins
    for (var i = 0; i < list.length; i++) {
      if (list[i]['_statewide'] === false) {
        markers.addLayer(markers_map[list[i][EMPLOYMENT_STAT_NID]]);
      }
    }
  }

  function draw_listing(list) {
    var listing_html = '';
    var regional_html = '';
    var statewide_html = '';
    var regional_count = 0;
    var statewide_count = 0;
    for (var i = 0; i < list.length; i++) {
      var is_statewide = list[i]['_statewide'];
      if (is_statewide) {
        statewide_count++;
        statewide_html += get_listing_item_html(list[i]);
      }
      else {
        regional_count++;
        regional_html += get_listing_item_html(list[i]);
      }
    }
    listing_html += '<div class="sticky-container">';
    listing_html += '<div class="listing sticky-front-container"></div>';
    listing_html += '<ul class="listing sticky-back-container">';
    if (regional_count > 0) {
      listing_html += '<li class="listing-header sticky"><span>Projects in this council (' + regional_count + ')</span></li>';
      listing_html += regional_html;
    }
    if (statewide_count > 0) {
      listing_html += '<li class="listing-header sticky"><span>Statewide projects (' + statewide_count + ')</span></li>';
      listing_html += statewide_html;
    }
    if (regional_count === 0 && statewide_count === 0) {
      listing_html += '<li><span class="listing-btn no-project-results">No projects could be found.</span></li>';
    }
    listing_html += '</ul></div>';
    $('.sidebar-content').html(listing_html);
    if (has_map) {
      $('.sticky-back-container').enscroll({
        scrollIncrement: 100,
        propagateWheelEvent: false
      });
      sticky_header.init();
    }
  }

  function region_list_window_resize(w) {
    if (has_map) {
      if (window_resize_functions.is('tablet_mobile')) {
        $('.sticky-back-container').enscroll('destroy');
        sticky_header.destroy();
      }
      else {
        $('.sticky-back-container').enscroll({
          scrollIncrement: 100,
          propagateWheelEvent: false
        });
        sticky_header.init();
      }
    }
  }

  function get_listing_item_html(program_item) {
    var is_statewide = program_item['_statewide'];
    var id = program_item[EMPLOYMENT_STAT_NID];
    var title = program_item[EMPLOYMENT_STAT_TITLE];
    var priority = get_priority_data_item(program_item[EMPLOYMENT_STAT_LOCATION_TITLE]);
    var expenditure = null;
    if (program_item[EMPLOYMENT_STAT_AVG_SALARY] !== null) {
      expenditure = trim_trailing_zeroes(program_item[EMPLOYMENT_STAT_AVG_SALARY]);
    }
    var has_priority = (priority !== null);

    var icon_html = '';
    if (is_statewide) {
      var state_pin_classes = 'statewide-pin' + ' ' + get_priority_color(program_item);
      icon_html = '<span class="' + state_pin_classes + '"></span>';
    }
    else {
      var i_o = markers_map[id].options.icon.options;
      var marker_classes = i_o.className + '-' + i_o.shape + '-' + i_o.markerColor + ' ' + i_o.className;
      icon_html = '<span class="' + marker_classes + '"><i number="' + i_o.number + '" style="color: ' + i_o.iconColor + '" class="' + i_o.icon + '"></i></span>';
    }
    var listing_data = '';
    if (has_priority) {
      listing_data += '<span class="priority ' + get_priority_color(program_item) + '">' + priority[PRIORITY_TITLE] + '</span>';
    }
    listing_data += '<h3>' + icon_html + title + '</h3>';
    if (expenditure !== null) {
      listing_data += '<span class="expenditure">' + expenditure + 'm</span>';
    }
    return '<li><button class="listing-btn" data-id="' + id + '">' + listing_data + '</button></li>';
  }

  function show_soft_loading(set_loading) {
    if (has_map) {
      if (set_loading) {
        $('.loading-screen').removeClass('hidden').addClass('soft').html('<span>Loading region...</span>');
      }
      else {
        $('.loading-screen').addClass('hidden').removeClass('soft').html('<span>Loading...</span>');
      }
    }
  }

  function draw_map_focus(word) {
    if (has_map) {
      if (word === "") {
        map.fitBounds(victorian_geodata_bounds, {
          animate: map_focus_animate
        });
      }
      else {
        // Focus on region
        var region_item = get_region_data_item(word);
        var lga = region_item[REGION_LGA_CODE];
        load_geodata_layer(lga, function(geo_layer) {
          if (geo_layer !== null) {
            map.fitBounds(geo_layer.getBounds(), {
              animate: map_focus_animate
            });
          }
        });
      }
    }
  }

  function draw_page_title(regional_id) {
    if (regional_id !== "") {
      var region = get_region_data_item(regional_id);
      $('.page__title').html('Getting it done in ' + region[REGION_TITLE]);
    }
    else {
      $('.page__title').html('Getting it done');
    }
  }

  // =========================================================
  // REGION - POPUP
  // =========================================================
  function open_popup(prog_data) {
    // Set the active area.
    $('.active-area').css('right', '320px');

    // Focus the Map.
    if (prog_data['_statewide']) {
      if ($('#region-filter').val()) {
        draw_map_focus($('#region-filter').val());
      }
      else {
        draw_map_focus("");
      }
    }
    else {
      var lat = prog_data[EMPLOYMENT_STAT_DATE];
      var lng = prog_data[EMPLOYMENT_STAT_FEM_SALARY];
      if (has_map) {
        map.setView([lat, lng], 15);
      }
    }

    // Load the program data into popup.
    var html = '';
    html += '<h3>' + prog_data[EMPLOYMENT_STAT_TITLE] + '</h3>';
    html += '<p>' + prog_data[EMPLOYMENT_STAT_BODY] + '</p>';
    if (prog_data[EMPLOYMENT_STAT_AVG_SALARY] !== null) {
      html += '<span class="expenditure">Total funding: ' + trim_trailing_zeroes(prog_data[EMPLOYMENT_STAT_AVG_SALARY]) + 'm</span>';
    }
    if (prog_data[EMPLOYMENT_STAT_MALE_TOT_EMP] !== '' && prog_data[EMPLOYMENT_STAT_LOC_ID] == 1 && prog_data[EMPLOYMENT_STAT_TOT_EMP] !== null) {
      html += '<a class="visit-page-link" href="' + prog_data[EMPLOYMENT_STAT_TOT_EMP] + '">Visit program page</a>';
    }
    $('.popup-content').html(html);

    // Remember if open.
    popup_showing_id = prog_data[EMPLOYMENT_STAT_NID];
    is_popup_open = true;

    // Set Highlight.
    $('.listing-btn').removeClass('highlight');
    var $btn = $('.listing-btn[data-id=' + popup_showing_id + ']');
    $btn.addClass('highlight');

    // Scroll button into view.
    var $sidebar = $('.sidebar-content .sticky-back-container');
    var header_height = 52;
    $sidebar.scrollTop($sidebar.scrollTop() + $btn.position().top - header_height);

    // Show popup.
    $('.popup').addClass('show');
    $('body').addClass('map-popup-showing');
  }

  function close_popup() {
    $('.listing-btn').removeClass('highlight');
    popup_showing_id = -1;
    is_popup_open = false;
    $('.active-area').css('right', '0');
    $('.popup').removeClass('show');
    $('body').removeClass('map-popup-showing');
    $('.popup-content').empty();
  }

  /**
   * Click function for map popup info button.
   */
  function info_button_click() {
    var id = $(this).data('id');
    if (is_popup_open && id == popup_showing_id) {
      close_popup();
    }
    else {
      open_popup(get_employment_statistics_item(id));
    }
  }

  function region_change_statewide_checkbox() {
    var $this = $(this);
    var id = $this.attr('id');
    var other_filter = (id === 'statewide-projects-bottom') ? '#statewide-projects-top' : '#statewide-projects-bottom';
    $(other_filter).prop('checked', $this.prop('checked'));
    region_change_filter();
  }

  // =========================================================
  // LISTENERS
  // =========================================================
  function region_add_listeners() {
    $('.sidebar-content').on('click', '.listing-btn', function() {
      var id = $(this).data('id');
      var prog_data = get_employment_statistics_item(id);
      if (prog_data['_statewide']) {
        // Show statewide details.
        open_popup(prog_data);
      }
      else {
        // Show program data details.
        open_popup(prog_data);
        markers_map[id].openPopup();
      }
    });
    $('.popup-close').on('click', close_popup);
    $('#map-display').on('click', '.info-btn', info_button_click);
    $('#region-filter').on('change', region_change_filter);
    $('#priorities-filter').on('change', region_change_filter);
    $('#statewide-projects-top, #statewide-projects-bottom').on('change', region_change_statewide_checkbox);
    $('#search-filter').on('keyup', region_change_filter);

    if (has_map) {
      window_resize_functions.add('region_list_tablet_in', region_list_window_resize, 'in', 'tablet_mobile');
      window_resize_functions.add('region_list_tablet_out', region_list_window_resize, 'out', 'tablet_mobile');
      window_resize_functions.init();
    }

    $('#zoom-in').on('click', function() {
      if (has_map) {
        map.zoomIn();
      }
    });
    $('#zoom-out').on('click', function() {
      if (has_map) {
        map.zoomOut();
      }
    });
    $('#reset-zoom').on('click', function() {
      draw_map_focus($('#region-filter').val());
    });
    $('.clear-prompt-cookie').on('click', function() {
      if (has_map) {
        $.cookie('download-map', 'no');
      }
      else {
        $.removeCookie('download-map');
      }
      location.reload();
    });
  }

  // =========================================================
  // INITIALIZATION
  // =========================================================
  function request_download_prompt() {
    // Check for cookie.
    if ($.cookie('download-map') !== undefined) {
      // Cookie exists. Don't bother with prompt.
      has_map = ($.cookie('download-map') === 'yes');
    }
    else {
      // Add Listeners.
      $('#view-map').on('click', download_prompt_received);
      $('#view-list').on('click', download_prompt_received);
    }
    initialize_controls_and_data();
  }

  function download_prompt_received(e) {
    // Set map.
    var element_id = $(this).attr('id');
    switch (element_id) {
      case 'view-map':
        $.cookie('download-map', 'yes');
        has_map = true;
        break;
      case 'view-list':
        $.cookie('download-map', 'no');
        has_map = false;
        break;
    }
    // Unload prompt.
    $('#view-map').off('click', download_prompt_received);
    $('#view-list').off('click', download_prompt_received);
    $('.download-prompt').remove();
    $('.loading-screen').removeClass('hidden');

    // Continue initialization.
    initialize_controls_and_data();
  }

  function initialize_controls_and_data() {
    // Begin loading map.
    region_initialize_controls();
    initialize_map();
    load_all_data(parse_employment_statistics);
  }

  // =========================================================
  // PUBLIC METHODS
  // =========================================================
  this.init = function() {
    initialize_controls_and_data();
  }
}

/**
 * @file
 * Explore Map / Region Map.
 * Initializes the Regional and Victorian Map.
 *
 */
(function($, Drupal, window, document, undefined) {
  Drupal.behaviors.explore_map = {
    attach: function(context, settings) {
      if ($('.map-widget').length > 0) {
        var map = null;
        if ($('.node-page').hasClass('regional-map')) {
          map = new RegionalMap();
        }
        else {
          map = new ExploreMap();
        }
        map.init();
      }
    }
  };
})(jQuery, Drupal, this, this.document);
