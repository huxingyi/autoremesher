"""Adds a label to a GitLab merge requests or issues."""

import os
import sys
import argparse
import requests

EIGEN_PROJECT_ID = 15462818  # Taken from the gitlab project page.


def add_label_to_mr(private_token: str, mr_iid: int, label: str):
    """
    Adds a label to a specific merge request in a GitLab project.

    Args:
        private_token: The user's private GitLab API token.
        mr_iid: The internal ID (IID) of the merge request.
        label: The label to add.
    """
    api_url = (
        f"https://gitlab.com/api/v4/projects/{EIGEN_PROJECT_ID}/merge_requests/{mr_iid}"
    )
    headers = {"PRIVATE-TOKEN": private_token}
    # Using 'add_labels' ensures we don't overwrite existing labels.
    payload = {"add_labels": label}

    try:
        response = requests.put(api_url, headers=headers, json=payload)
        response.raise_for_status()  # Raises an HTTPError for bad responses (4xx or 5xx)
        print(f"✅ Successfully added label '{label}' to Merge Request !{mr_iid}.")
    except requests.exceptions.RequestException as e:
        print(f"❌ Error updating Merge Request !{mr_iid}: {e}", file=sys.stderr)
        if hasattr(e, "response") and e.response is not None:
            print(f"   Response: {e.response.text}", file=sys.stderr)


def add_label_to_issue(private_token: str, issue_iid: int, label: str):
    """
    Adds a label to a specific issue in a GitLab project.

    Args:
        private_token: The user's private GitLab API token.
        issue_iid: The internal ID (IID) of the issue.
        label: The label to add.
    """
    api_url = (
        f"https://gitlab.com/api/v4/projects/{EIGEN_PROJECT_ID}/issues/{issue_iid}"
    )
    headers = {"PRIVATE-TOKEN": private_token}
    payload = {"add_labels": label}

    try:
        response = requests.put(api_url, headers=headers, json=payload)
        response.raise_for_status()
        print(f"✅ Successfully added label '{label}' to Issue #{issue_iid}.")
    except requests.exceptions.RequestException as e:
        print(f"❌ Error updating Issue #{issue_iid}: {e}", file=sys.stderr)
        if hasattr(e, "response") and e.response is not None:
            print(f"   Response: {e.response.text}", file=sys.stderr)


def main():
    """
    Main function to parse arguments and trigger the labelling process.
    """
    parser = argparse.ArgumentParser(
        description="Add a label to GitLab merge requests and issues.",
        formatter_class=argparse.RawTextHelpFormatter,
    )
    parser.add_argument("label", help="The label to add.")
    parser.add_argument(
        "--mrs",
        nargs="+",
        type=int,
        help="A space-separated list of Merge Request IIDs.",
    )
    parser.add_argument(
        "--issues", nargs="+", type=int, help="A space-separated list of Issue IIDs."
    )
    parser.add_argument(
        "--gitlab_private_token",
        help="Your GitLab private access token. \n(Best practice is to use the GITLAB_PRIVATE_TOKEN environment variable instead.)",
    )
    args = parser.parse_args()

    # Prefer environment variable for the token for better security.
    gitlab_private_token = args.gitlab_private_token or os.environ.get(
        "GITLAB_PRIVATE_TOKEN"
    )
    if not gitlab_private_token:
        print("Error: GitLab private token not found.", file=sys.stderr)
        print(
            "Please provide it using the --token argument or by setting the GITLAB_PRIVATE_TOKEN environment variable.",
            file=sys.stderr,
        )
        sys.exit(1)

    if not args.mrs and not args.issues:
        print(
            "Error: You must provide at least one merge request (--mrs) or issue (--issues) ID.",
            file=sys.stderr,
        )
        sys.exit(1)

    print("-" * 30)

    if args.mrs:
        print(f"Processing {len(args.mrs)} merge request(s)...")
        for mr_iid in args.mrs:
            add_label_to_mr(gitlab_private_token, mr_iid, args.label)

    if args.issues:
        print(f"\nProcessing {len(args.issues)} issue(s)...")
        for issue_iid in args.issues:
            add_label_to_issue(gitlab_private_token, issue_iid, args.label)

    print("-" * 30)
    print("Script finished.")


if __name__ == "__main__":
    main()
