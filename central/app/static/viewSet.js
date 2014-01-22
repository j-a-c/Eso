// Dropdown menu for selecting entity type
var entityTypeSelect = '<select><option value="1">'+getEntityTypeString(1)+'</option><option value="2">'+getEntityTypeString(2)+'</option></select>'

// Multiselect menu for selecting operation
var operationSelect = '<select multiple><option value="1">Retrieve</option><option value="2">Sign</option><option value="4">Verify</option><option value="8">Encrypt</option><option value="16">Decrypt</option><option value="32">HMAC</option></select>'

/*
 * Utility function to map entity types numbers to values.
 */
function getEntityTypeString(typeNumber)
{
	switch(typeNumber)
	{
	case 1:
		return 'User';
	case 2:
		return 'POSIX Group';
	}
}

/*
 * Attach a function to the 'Edit' buttons.
 * Current only allows operation to be updated.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.editPermButton', function(){ 
	var currentRow 		= $(this).closest('tr');

	// Replace 'Operation' contents with a multiselect menu.
	var opOptions 		= currentRow.find('td').eq(2).html(operationSelect);

	// Update button function to 'update'.
	$(this).attr('class', 'updatePermButton');
	$(this).text('Save');

});

/*
 * Attach a function to the 'Save' buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.updatePermButton', function(){
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');
	var entityRow	 	= currentRow.find('td').eq(0);
	var opOptions 		= currentRow.find('select').eq(0);
	var locRow 			= currentRow.find('td').eq(3);

	// Calculate the operations allowed for this entity (ops)
    var ops = 0;
    opOptions.children('option').each(function(){
		if($(this).is(':selected'))
		{
			ops += parseInt($(this).val());
		}
	});

	$.getJSON($SCRIPT_ROOT + '/_update_perm', {
		setName: 		$("#setName").html(),
        entity: 		entityRow.html(),
		op:				ops,
		loc: 			locRow.html()
      }, function(data) {
		// TODO if result is true, hide input and replace with values.
		// data.result will be 0 on success.
        if(!data.result)
		{
			// Update button function to 'edit'.
			// Show new operation values.
			currentButton.attr('class', 'editPermButton');
			currentButton.text('Edit');
			currentRow.find('td').eq(2).html(ops);
		}
		else
		{
			// TODO handle error
			alert('Error updating permission: ' + data.result);
		}
	});
});

/*
 * Attach a function to the "Create" buttons.
 * Need to use this because the button will be added dynamically.
 */
$(document).on('click', '.createPermButton', function(){
	var currentButton 	= $(this);

	var currentRow 		= $(this).closest('tr');
	var entityRow	 	= currentRow.find('input').eq(0);
	var entityOptions 	= currentRow.find('select').eq(0);
	var opOptions 		= currentRow.find('select').eq(1);
	var locRow 			= currentRow.find('input').eq(1);

	// Calculate the operations allowed for this entity (ops)
    var ops = 0;
    opOptions.children('option').each(function(){
		if($(this).is(':selected'))
		{
			ops += parseInt($(this).val());
		}
	});

	$.getJSON($SCRIPT_ROOT + '/_create_perm', {
		setName: 		$("#setName").html(),
        entity: 		entityRow.val(),
        entity_type: 	entityOptions.find(":selected").val(),
		op:				ops,
		loc: 			locRow.val()
      }, function(data) {
		// TODO if result is true, hide input and replace with values.
		// data.result will be 0 on success.
        if(!data.result)
		{
			// Update row values.
			currentRow.find('td').eq(0).html(entityRow.val());
			currentRow.find('td').eq(1).html(entityOptions.find(":selected").val());
			currentRow.find('td').eq(2).html(ops);
			currentRow.find('td').eq(3).html(locRow.val());

			// Update button function to 'edit'.
			currentButton.attr('class', 'editPermButton');
			currentButton.text('Edit');
		}
		else
		{
			// TODO handle error
			alert('Error creating permission: ' + data.result);
		}
	});

});

/*
 * Occurs when the "Add New Permission" button is clicked.
 * Adds a new row to the table.
 */
function addPermRow()
{
	$('#viewPermissions tr:last').after('<tr><td><input type="text"/></td><td>'+entityTypeSelect+'</td><td>'+operationSelect+'</td><td><input type="text"/></td><td><button class="createPermButton">Create</button></td></tr>');


}
