from flask import Flask, request, redirect, url_for, render_template
from databaseModule import *
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
    if request.method == 'POST':

        # Get form contents.
        setName = request.form.get('setName')
        version = request.form.get('version')
        expiration = request.form.get('expiration')
        # TODO credential type
        primary = request.form.get('primary')
        secondary = request.form.get('secondary')

        # Are all the contents of the form 'clean'?
        # (In the expected form and satisfy the constraints.)
        isClean = True

        # Check set name.
        if not re.match('^\w+(\.\w+)*$', setName):
            isClean = False

        # Check version.
        # Must be an integer greater than 0.
        if (not version.isdigit()) or (int(version) <= 0):
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

        if create_set(setName, version, expiration, primary, secondary) == 0:
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

    # TODO add option to edit
	return render_template('viewSet.html', setName=setName)


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
