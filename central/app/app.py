from flask import Flask, jsonify, request, redirect, url_for, render_template
from appExtension import *
import datetime
import re

app = Flask(__name__)


'''
    The 'home page' for the central authority admin interface.
'''
@app.route('/', methods=['GET', 'POST'])
def index():
    if request.method == 'POST':
        # View a set.
        if request.form['submit'] == 'View Set':
            setName = request.form.get('setName')
            return redirect('set/' + setName)
        # Create a set.
        elif request.form['submit'] == 'Create Set':
            return redirect(url_for('createSet'))
    return render_template('index.html')


'''
    Form for creating a new set.
'''
@app.route('/create/', methods=['GET', 'POST'])
def createSet():
    # Maps the the selected credential type to its algorithm, key size, 
    # and type. Type 1 = credential, 2 = symmetric, 3 = asymmetric.
    # See db_types.h
    # TODO set a safe password size
    credMap = { 'Username / Password'   : ('PASS',  '33', '1'),
                'AES-128'               : ('AES',  '128', '2'),
                'AES-256'               : ('AES',  '256', '2'),
                'RSA-1024'              : ('RSA', '1024', '3'),
                'RSA-2048'              : ('RSA', '2048', '3'),
                'RSA-4096'              : ('RSA', '4096', '3')
            }

    if request.method == 'POST':

        # Get form contents.
        setName = request.form.get('setName')
        version = request.form.get('version')
        (credAlgo, credSize, credType) = credMap[request.form.get('credType')]
        expiration = request.form.get('expiration')
        primary = request.form.get('primary')
        secondary = request.form.get('secondary')

        # Are all the contents of the form 'clean'?
        # (In the expected form and satisfy the constraints.)
        isClean = True

        # Check set name.
        if not re.match('^\w+(\.\w+)*$', setName):
            isClean = False

        # Check version.
        # Must be an integer greater than 0 and less than 4294967295 .
        if (not version.isdigit()) or (int(version) <= 0 or int(version) >= 4294967295):
            isClean = False

        # Check expiration.
        # Must be of the form YYYY-MM-DD
        try:
            datetime.datetime.strptime(expiration, '%Y-%m-%d')
        except ValueError:
            isClean = False

        # Check primary.
        # Must be a name with no special characters.
        if not re.match('^\w+$', primary):
            isClean = False

        # Check secondary.
        # Must be a name with no special characters.
        if not re.match('^\w+$', secondary):
            isClean = False

        if not isClean:
            # TODO Error message
            return render_template('createSet.html')

        # Was the set actually created?
        isCreated = False

        # TODO handle algo, size, etc.
        # (credAlgo, credSize, credType)
        if create_credential(setName, version, expiration, primary, secondary,
                credType, credAlgo, credSize) == 0:
            isCreated = True

        if not isCreated:
            # TODO Error message and save form input.
            return render_template('createSet.html')

        # If set was created, redirect to its page.
        return redirect('set/' + setName)

    return render_template('createSet.html')


'''
    Page for viewing a set.
'''
@app.route('/set/<setName>/')
def viewSet(setName):

    # Load info from database
    setCreds = get_credentials(setName)
    setPerms = get_all_permissions(setName)

    # TODO add option to edit
    return render_template('viewSet.html', setName=setName, setCreds=setCreds,
            setPerms=setPerms)


'''
    Attempt to create a permission.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_create_perm')
def createPermission():
    set_name = request.args.get('setName', '', type=str)
    entity = request.args.get('entity', '', type=str)
    entity_type = request.args.get('entity_type', '', type=str)
    op = request.args.get('op', '', type=str)

    # Validate input

    # Check entity.
    # Must be a name with no special characters.
    if not re.match('^\w+$', entity):
        return jsonify(result=6)

    # create_permission(...) returns 0 on success, so will we.
    if (create_permission(set_name, entity, entity_type, op)):
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)


'''
    Attempt to update a permission.
    Will result json {result:0} if everything went ok and {result:1} otherwise.
'''
@app.route('/_update_perm')
def updatePermission():
    set_name = request.args.get('setName', '', type=str)
    entity = request.args.get('entity', '', type=str)
    op = request.args.get('op', '', type=str)

    # update_permission(...) returns 0 on success, so will we.
    if (update_permission(set_name, entity, op)):
        # Something went wrong.
        return jsonify(result=1)
    else:
        # Everything went ok!
        return jsonify(result=0)



"""
'''
    Sample login page.
'''
@app.route('/login/', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        username = request.form.get('username')
        password = request.form.get('username')

        # TODO process login
        return redirect(url_for('greet', name=username))

    return render_template('login.html')
"""

if __name__ == '__main__':
    app.run(debug=True)
